#include "ContentLoader.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <string>

#include <coders/json.hpp>
#include <core_defs.hpp>
#include "../data/dynamic.hpp"
#include <debug/Logger.hpp>
#include <files/files.hpp>
#include <items/ItemDef.hpp>
#include <logic/scripting/scripting.hpp>
#include <objects/rigging.hpp>
#include <typedefs.hpp>
#include <util/listutil.hpp>
#include <util/stringutil.hpp>
#include <voxels/Block.hpp>
#include "Content.hpp"
#include "ContentBuilder.hpp"
#include "ContentPack.hpp"

namespace fs = std::filesystem;

static debug::Logger logger("content-loader");

ContentLoader::ContentLoader(ContentPack* pack, ContentBuilder& builder)
    : pack(pack), builder(builder) {
    auto runtime = std::make_unique<ContentPackRuntime>(
        *pack, scripting::create_pack_environment(*pack)
    );
    stats = &runtime->getStatsWriteable();
    env = runtime->getEnvironment();
    this->runtime = runtime.get();
    builder.add(std::move(runtime));
}

static void detect_defs(
    const fs::path& folder,
    const std::string& prefix,
    std::vector<std::string>& detected
) {
    if (fs::is_directory(folder)) {
        for (const auto& entry : fs::directory_iterator(folder)) {
            const fs::path& file = entry.path();
            std::string name = file.stem().string();
            if (name[0] == '_') {
                continue;
            }
            if (fs::is_regular_file(file) && file.extension() == ".json") {
                detected.push_back(prefix.empty() ? name : prefix + ":" + name);
            } else if (fs::is_directory(file)) {
                detect_defs(file, name, detected);
            }
        }
    }
}

bool ContentLoader::fixPackIndices(
    const fs::path& folder,
    dynamic::Map* indicesRoot,
    const std::string& contentSection
) {
    std::vector<std::string> detected;
    detect_defs(folder, "", detected);

    std::vector<std::string> indexed;
    bool modified = false;
    if (!indicesRoot->has(contentSection)) {
        indicesRoot->putList(contentSection);
    }
    auto arr = indicesRoot->list(contentSection);
    if (arr) {
        for (uint i = 0; i < arr->size(); i++) {
            std::string name = arr->str(i);
            if (!util::contains(detected, name)) {
                arr->remove(i);
                i--;
                modified = true;
                continue;
            }
            indexed.push_back(name);
        }
    }
    for (auto name : detected) {
        if (!util::contains(indexed, name)) {
            arr->put(name);
            modified = true;
        }
    }
    return modified;
}

void ContentLoader::fixPackIndices() {
    auto folder = pack->folder;
    auto indexFile = pack->getContentFile();
    auto blocksFolder = folder / ContentPack::BLOCKS_FOLDER;
    auto itemsFolder = folder / ContentPack::ITEMS_FOLDER;
    auto entitiesFolder = folder / ContentPack::ENTITIES_FOLDER;

    dynamic::Map_sptr root;
    if (fs::is_regular_file(indexFile)) {
        root = files::read_json(indexFile);
    } else {
        root = dynamic::create_map();
    }

    bool modified = false;
    modified |= fixPackIndices(blocksFolder, root.get(), "blocks");
    modified |= fixPackIndices(itemsFolder, root.get(), "items");
    modified |= fixPackIndices(entitiesFolder, root.get(), "entities");

    if (modified) {
        // rewrite modified json
        files::write_json(indexFile, root.get());
    }
}

void ContentLoader::loadBlock(
    Block& def, const std::string& name, const fs::path& file
) {
    auto root = files::read_json(file);

    root->str("caption", def.caption);

    // block texturing
    if (root->has("texture")) {
        std::string texture;
        root->str("texture", texture);
        for (uint i = 0; i < 6; i++) {
            def.textureFaces[i] = texture;
        }
    } else if (root->has("texture-faces")) {
        auto texarr = root->list("texture-faces");
        for (uint i = 0; i < 6; i++) {
            def.textureFaces[i] = texarr->str(i);
        }
    }

    // block model
    std::string modelName;
    root->str("model", modelName);
    if (auto model = BlockModel_from(modelName)) {
        if (*model == BlockModel::custom) {
            if (root->has("model-primitives")) {
                loadCustomBlockModel(def, root->map("model-primitives").get());
            } else {
                logger.error() << name << ": no 'model-primitives' found";
            }
        }
        def.model = *model;
    } else if (!modelName.empty()) {
        logger.error() << "unknown model " << modelName;
        def.model = BlockModel::none;
    }

    root->str("material", def.material);

    // rotation profile
    std::string profile = "none";
    root->str("rotation", profile);
    def.rotatable = profile != "none";
    if (profile == BlockRotProfile::PIPE_NAME) {
        def.rotations = BlockRotProfile::PIPE;
    } else if (profile == BlockRotProfile::PANE_NAME) {
        def.rotations = BlockRotProfile::PANE;
    } else if (profile != "none") {
        logger.error() << "unknown rotation profile " << profile;
        def.rotatable = false;
    }

    // block hitbox AABB [x, y, z, width, height, depth]
    auto boxarr = root->list("hitboxes");
    if (boxarr) {
        def.hitboxes.resize(boxarr->size());
        for (uint i = 0; i < boxarr->size(); i++) {
            auto box = boxarr->list(i);
            auto& hitboxesIndex = def.hitboxes[i];
            hitboxesIndex.a = glm::vec3(box->num(0), box->num(1), box->num(2));
            hitboxesIndex.b = glm::vec3(box->num(3), box->num(4), box->num(5));
            hitboxesIndex.b += hitboxesIndex.a;
        }
    } else if ((boxarr = root->list("hitbox"))) {
        AABB aabb;
        aabb.a = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
        aabb.b = glm::vec3(boxarr->num(3), boxarr->num(4), boxarr->num(5));
        aabb.b += aabb.a;
        def.hitboxes = {aabb};
    } else if (!def.modelBoxes.empty()) {
        def.hitboxes = def.modelBoxes;
    } else {
        def.hitboxes = {AABB()};
    }

    // block light emission [r, g, b] where r,g,b in range [0..15]
    if (auto emissionarr = root->list("emission")) {
        def.emission[0] = emissionarr->num(0);
        def.emission[1] = emissionarr->num(1);
        def.emission[2] = emissionarr->num(2);
    }

    // block size
    if (auto sizearr = root->list("size")) {
        def.size.x = sizearr->num(0);
        def.size.y = sizearr->num(1);
        def.size.z = sizearr->num(2);
        if (def.model == BlockModel::block &&
            (def.size.x != 1 || def.size.y != 1 || def.size.z != 1)) {
            def.model = BlockModel::aabb;
            def.hitboxes = {AABB(def.size)};
        }
    }

    // primitive properties
    root->flag("obstacle", def.obstacle);
    root->flag("replaceable", def.replaceable);
    root->flag("light-passing", def.lightPassing);
    root->flag("sky-light-passing", def.skyLightPassing);
    root->flag("shadeless", def.shadeless);
    root->flag("ambient-occlusion", def.ambientOcclusion);
    root->flag("breakable", def.breakable);
    root->flag("selectable", def.selectable);
    root->flag("grounded", def.grounded);
    root->flag("hidden", def.hidden);
    root->num("draw-group", def.drawGroup);
    root->str("picking-item", def.pickingItem);
    root->str("script-name", def.scriptName);
    root->str("ui-layout", def.uiLayout);
    root->num("inventory-size", def.inventorySize);
    root->num("tick-interval", def.tickInterval);
    if (def.tickInterval == 0) {
        def.tickInterval = 1;
    }

    if (def.hidden && def.pickingItem == def.name + BLOCK_ITEM_SUFFIX) {
        def.pickingItem = CORE_EMPTY;
    }
}

void ContentLoader::loadCustomBlockModel(Block& def, dynamic::Map* primitives) {
    if (primitives->has("aabbs")) {
        auto modelboxes = primitives->list("aabbs");
        for (uint i = 0; i < modelboxes->size(); i++) {
            /* Parse aabb */
            auto boxarr = modelboxes->list(i);
            AABB modelbox;
            modelbox.a =
                glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
            modelbox.b =
                glm::vec3(boxarr->num(3), boxarr->num(4), boxarr->num(5));
            modelbox.b += modelbox.a;
            def.modelBoxes.push_back(modelbox);

            if (boxarr->size() == 7)
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.emplace_back(boxarr->str(6));
                }
            else if (boxarr->size() == 12)
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.emplace_back(boxarr->str(j));
                }
            else
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.emplace_back("notfound");
                }
        }
    }
    if (primitives->has("tetragons")) {
        auto modeltetragons = primitives->list("tetragons");
        for (uint i = 0; i < modeltetragons->size(); i++) {
            /* Parse tetragon to points */
            auto tgonobj = modeltetragons->list(i);
            glm::vec3 p1(tgonobj->num(0), tgonobj->num(1), tgonobj->num(2)),
                xw(tgonobj->num(3), tgonobj->num(4), tgonobj->num(5)),
                yh(tgonobj->num(6), tgonobj->num(7), tgonobj->num(8));
            def.modelExtraPoints.push_back(p1);
            def.modelExtraPoints.push_back(p1 + xw);
            def.modelExtraPoints.push_back(p1 + xw + yh);
            def.modelExtraPoints.push_back(p1 + yh);

            def.modelTextures.emplace_back(tgonobj->str(9));
        }
    }
}

void ContentLoader::loadItem(
    ItemDef& def, const std::string& name, const fs::path& file
) {
    auto root = files::read_json(file);
    root->str("caption", def.caption);

    std::string iconTypeStr = "";
    root->str("icon-type", iconTypeStr);
    if (iconTypeStr == "none") {
        def.iconType = item_icon_type::none;
    } else if (iconTypeStr == "block") {
        def.iconType = item_icon_type::block;
    } else if (iconTypeStr == "sprite") {
        def.iconType = item_icon_type::sprite;
    } else if (iconTypeStr.length()) {
        logger.error() << name << ": unknown icon type" << iconTypeStr;
    }
    root->str("icon", def.icon);
    root->str("placing-block", def.placingBlock);
    root->str("script-name", def.scriptName);
    root->num("stack-size", def.stackSize);

    // item light emission [r, g, b] where r,g,b in range [0..15]
    if (auto emissionarr = root->list("emission")) {
        def.emission[0] = emissionarr->num(0);
        def.emission[1] = emissionarr->num(1);
        def.emission[2] = emissionarr->num(2);
    }
}

void ContentLoader::loadEntity(
    EntityDef& def, const std::string& name, const fs::path& file
) {
    auto root = files::read_json(file);
    if (auto componentsarr = root->list("components")) {
        for (size_t i = 0; i < componentsarr->size(); i++) {
            def.components.emplace_back(componentsarr->str(i));
        }
    }
    if (auto boxarr = root->list("hitbox")) {
        def.hitbox = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
    }
    if (auto sensorsarr = root->list("sensors")) {
        for (size_t i = 0; i < sensorsarr->size(); i++) {
            if (auto sensorarr = sensorsarr->list(i)) {
                auto sensorType = sensorarr->str(0);
                if (sensorType == "aabb") {
                    def.boxSensors.emplace_back(
                        i,
                        AABB {
                            {sensorarr->num(1),
                             sensorarr->num(2),
                             sensorarr->num(3)},
                            {sensorarr->num(4),
                             sensorarr->num(5),
                             sensorarr->num(6)}}
                    );
                } else if (sensorType == "radius") {
                    def.radialSensors.emplace_back(i, sensorarr->num(1));
                } else {
                    logger.error()
                        << name << ": sensor #" << i << " - unknown type "
                        << util::quote(sensorType);
                }
            }
        }
    }
    root->flag("save", def.save.enabled);
    root->flag("save-skeleton-pose", def.save.skeleton.pose);
    root->flag("save-skeleton-textures", def.save.skeleton.textures);
    root->flag("save-body-velocity", def.save.body.velocity);
    root->flag("save-body-settings", def.save.body.settings);

    std::string bodyTypeName;
    root->str("body-type", bodyTypeName);
    if (auto bodyType = BodyType_from(bodyTypeName)) {
        def.bodyType = *bodyType;
    }

    root->str("skeleton-name", def.skeletonName);
    root->flag("blocking", def.blocking);
}

void ContentLoader::loadEntity(
    EntityDef& def, const std::string& full, const std::string& name
) {
    auto folder = pack->folder;
    auto configFile = folder / fs::path("entities/" + name + ".json");
    if (fs::exists(configFile)) loadEntity(def, full, configFile);
}

void ContentLoader::loadBlock(
    Block& def, const std::string& full, const std::string& name
) {
    auto folder = pack->folder;
    auto configFile = folder / fs::path("blocks/" + name + ".json");
    if (fs::exists(configFile)) loadBlock(def, full, configFile);

    auto scriptfile = folder / fs::path("scripts/" + def.scriptName + ".lua");
    if (fs::is_regular_file(scriptfile)) {
        scripting::load_block_script(env, full, scriptfile, def.rt.funcsset);
    }
    if (!def.hidden) {
        auto& item = builder.items.create(full + BLOCK_ITEM_SUFFIX);
        item.generated = true;
        item.caption = def.caption;
        item.iconType = item_icon_type::block;
        item.icon = full;
        item.placingBlock = full;

        for (uint j = 0; j < 4; j++) {
            item.emission[j] = def.emission[j];
        }
        stats->totalItems++;
    }
}

void ContentLoader::loadItem(
    ItemDef& def, const std::string& full, const std::string& name
) {
    auto folder = pack->folder;
    auto configFile = folder / fs::path("items/" + name + ".json");
    if (fs::exists(configFile)) loadItem(def, full, configFile);

    auto scriptfile = folder / fs::path("scripts/" + def.scriptName + ".lua");
    if (fs::is_regular_file(scriptfile)) {
        scripting::load_item_script(env, full, scriptfile, def.rt.funcsset);
    }
}

void ContentLoader::loadBlockMaterial(
    BlockMaterial& def, const fs::path& file
) {
    auto root = files::read_json(file);
    root->str("steps-sound", def.stepsSound);
    root->str("place-sound", def.placeSound);
    root->str("break-sound", def.breakSound);
}

void ContentLoader::load() {
    logger.info() << "loading pack [" << pack->id << "]";

    fixPackIndices();

    auto folder = pack->folder;

    fs::path scriptFile = folder / fs::path("scripts/world.lua");
    if (fs::is_regular_file(scriptFile)) {
        scripting::load_world_script(
            env, pack->id, scriptFile, runtime->worldfuncsset
        );
    }

    if (!fs::is_regular_file(pack->getContentFile())) return;

    auto root = files::read_json(pack->getContentFile());

    if (auto blocksarr = root->list("blocks")) {
        for (size_t i = 0; i < blocksarr->size(); i++) {
            std::string name = blocksarr->str(i);
            auto colon = name.find(':');
            std::string full =
                colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.blocks.create(full);
            if (colon != std::string::npos)
                def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            loadBlock(def, full, name);
            stats->totalBlocks++;
        }
    }
    if (auto itemsarr = root->list("items")) {
        for (size_t i = 0; i < itemsarr->size(); i++) {
            std::string name = itemsarr->str(i);
            auto colon = name.find(':');
            std::string full =
                colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.items.create(full);
            if (colon != std::string::npos)
                def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            loadItem(def, full, name);
            stats->totalItems++;
        }
    }

    if (auto entitiesarr = root->list("entities")) {
        for (size_t i = 0; i < entitiesarr->size(); i++) {
            std::string name = entitiesarr->str(i);
            auto colon = name.find(':');
            std::string full =
                colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.entities.create(full);
            loadEntity(def, full, name);
            stats->totalEntities++;
        }
    }

    fs::path materialsDir = folder / fs::u8path("block_materials");
    if (fs::is_directory(materialsDir)) {
        for (const auto& entry : fs::directory_iterator(materialsDir)) {
            const fs::path& file = entry.path();
            std::string name = pack->id + ":" + file.stem().u8string();
            loadBlockMaterial(builder.createBlockMaterial(name), file);
        }
    }

    fs::path skeletonsDir = folder / fs::u8path("skeletons");
    if (fs::is_directory(skeletonsDir)) {
        for (const auto& entry : fs::directory_iterator(skeletonsDir)) {
            const fs::path& file = entry.path();
            std::string name = pack->id + ":" + file.stem().u8string();
            std::string text = files::read_string(file);
            builder.add(
                rigging::SkeletonConfig::parse(text, file.u8string(), name)
            );
        }
    }

    fs::path componentsDir = folder / fs::u8path("scripts/components");
    if (fs::is_directory(componentsDir)) {
        for (const auto& entry : fs::directory_iterator(componentsDir)) {
            fs::path scriptfile = entry.path();
            if (fs::is_regular_file(scriptfile)) {
                auto name = pack->id + ":" + scriptfile.stem().u8string();
                scripting::load_entity_component(name, scriptfile);
            }
        }
    }

    fs::path resourcesFile = folder / fs::u8path("resources.json");
    if (fs::exists(resourcesFile)) {
        auto resRoot = files::read_json(resourcesFile);
        for (const auto& [key, _] : resRoot->values) {
            if (auto resType = ResourceType_from(key)) {
                if (auto arr = resRoot->list(key)) {
                    loadResources(*resType, arr.get());
                }
            } else {
                logger.warning() << "unknown resource type: " << key;
            }
        }
    }
}

void ContentLoader::loadResources(ResourceType type, dynamic::List* list) {
    for (size_t i = 0; i < list->size(); i++) {
        builder.resourceIndices[static_cast<size_t>(type)].add(
            pack->id + ":" + list->str(i), nullptr
        );
    }
}
