#include "ContentLoader.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "Content.hpp"
#include "ContentBuilder.hpp"
#include "ContentPack.hpp"
#include "coders/json.hpp"
#include "core_defs.hpp"
#include "debug/Logger.hpp"
#include "files/files.hpp"
#include "items/ItemDef.hpp"
#include "logic/scripting/scripting.hpp"
#include "objects/rigging.hpp"
#include "typedefs.hpp"
#include "util/listutil.hpp"
#include "util/stringutil.hpp"
#include "voxels/Block.hpp"
#include "data/dv_util.hpp"
#include "data/StructLayout.hpp"

namespace fs = std::filesystem;
using namespace data;

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
    dv::value& indicesRoot,
    const std::string& contentSection
) {
    std::vector<std::string> detected;
    detect_defs(folder, "", detected);

    std::vector<std::string> indexed;
    bool modified = false;
    if (!indicesRoot.has(contentSection)) {
        indicesRoot.list(contentSection);
    }
    auto& arr = indicesRoot[contentSection];
    for (size_t i = 0; i < arr.size(); i++) {
        const std::string& name = arr[i].asString();
        if (!util::contains(detected, name)) {
            arr.erase(i);
            i--;
            modified = true;
            continue;
        }
        indexed.push_back(name);
    }
    for (auto name : detected) {
        if (!util::contains(indexed, name)) {
            arr.add(name);
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

    dv::value root;
    if (fs::is_regular_file(indexFile)) {
        root = files::read_json(indexFile);
    } else {
        root = dv::object();
    }

    bool modified = false;
    modified |= fixPackIndices(blocksFolder, root, "blocks");
    modified |= fixPackIndices(itemsFolder, root, "items");
    modified |= fixPackIndices(entitiesFolder, root, "entities");

    if (modified) {
        // rewrite modified json
        files::write_json(indexFile, root);
    }
}

static void perform_user_block_fields(
    const std::string& blockName, StructLayout& layout
) {
    if (layout.size() > MAX_USER_BLOCK_FIELDS_SIZE) {
        throw std::runtime_error(
            util::quote(blockName) + 
            " fields total size exceeds limit (" + 
            std::to_string(layout.size()) + "/" +
            std::to_string(MAX_USER_BLOCK_FIELDS_SIZE) + ")");
    }
    for (const auto& field : layout) {
        if (field.name.at(0) == '.') {
            throw std::runtime_error(
                util::quote(blockName) + " field " + field.name + 
                ": user field may not start with '.'");
        }
    }

    std::vector<Field> fields;
    fields.insert(fields.end(), layout.begin(), layout.end());
    // add built-in fields here
    layout = StructLayout::create(fields);
}

void ContentLoader::loadBlock(
    Block& def, const std::string& name, const fs::path& file
) {
    auto root = files::read_json(file);

    if (root.has("parent")) {
        const auto& parentName = root["parent"].asString();
        auto parentDef = this->builder.blocks.get(parentName);
        if (parentDef == nullptr) {
            throw std::runtime_error(
                "Failed to find parent(" + parentName + ") for " + name
            );
        }
        parentDef->cloneTo(def);
    }

    root.at("caption").get(def.caption);

    // block texturing
    if (root.has("texture")) {
        const auto& texture = root["texture"].asString();
        for (uint i = 0; i < 6; i++) {
            def.textureFaces[i] = texture;
        }
    } else if (root.has("texture-faces")) {
        const auto& texarr = root["texture-faces"];
        for (uint i = 0; i < 6; i++) {
            def.textureFaces[i] = texarr[i].asString();
        }
    }

    // block model
    std::string modelName;
    root.at("model").get(modelName);
    if (auto model = BlockModel_from(modelName)) {
        if (*model == BlockModel::custom) {
            if (root.has("model-primitives")) {
                loadCustomBlockModel(def, root["model-primitives"]);
            } else {
                logger.error() << name << ": no 'model-primitives' found";
            }
        }
        def.model = *model;
    } else if (!modelName.empty()) {
        logger.error() << "unknown model " << modelName;
        def.model = BlockModel::none;
    }

    root.at("material").get(def.material);

    // rotation profile
    std::string profile = "none";
    root.at("rotation").get(profile);

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
    if (auto found = root.at("hitboxes")) {
        const auto& boxarr = *found;
        def.hitboxes.resize(boxarr.size());
        for (uint i = 0; i < boxarr.size(); i++) {
            const auto& box = boxarr[i];
            auto& hitboxesIndex = def.hitboxes[i];
            hitboxesIndex.a = glm::vec3(
                box[0].asNumber(), box[1].asNumber(), box[2].asNumber()
            );
            hitboxesIndex.b = glm::vec3(
                box[3].asNumber(), box[4].asNumber(), box[5].asNumber()
            );
            hitboxesIndex.b += hitboxesIndex.a;
        }
    } else if (auto found = root.at("hitbox")) {
        const auto& box = *found;
        AABB aabb;
        aabb.a = glm::vec3(
            box[0].asNumber(), box[1].asNumber(), box[2].asNumber()
        );
        aabb.b = glm::vec3(
            box[3].asNumber(), box[4].asNumber(), box[5].asNumber()
        );
        aabb.b += aabb.a;
        def.hitboxes = {aabb};
    } else if (!def.modelBoxes.empty()) {
        def.hitboxes = def.modelBoxes;
    } else {
        def.hitboxes = {AABB()};
    }

    // block light emission [r, g, b] where r,g,b in range [0..15]
    if (auto found = root.at("emission")) {
        const auto& emissionarr = *found;
        def.emission[0] = emissionarr[0].asNumber();
        def.emission[1] = emissionarr[1].asNumber();
        def.emission[2] = emissionarr[2].asNumber();
    }

    // block size
    if (auto found = root.at("size")) {
        const auto& sizearr = *found;
        def.size.x = sizearr[0].asNumber();
        def.size.y = sizearr[1].asNumber();
        def.size.z = sizearr[2].asNumber();
        if (def.model == BlockModel::block &&
            (def.size.x != 1 || def.size.y != 1 || def.size.z != 1)) {
            def.model = BlockModel::aabb;
            def.hitboxes = {AABB(def.size)};
        }
    }

    // primitive properties
    root.at("obstacle").get(def.obstacle);
    root.at("replaceable").get(def.replaceable);
    root.at("light-passing").get(def.lightPassing);
    root.at("sky-light-passing").get(def.skyLightPassing);
    root.at("shadeless").get(def.shadeless);
    root.at("ambient-occlusion").get(def.ambientOcclusion);
    root.at("breakable").get(def.breakable);
    root.at("selectable").get(def.selectable);
    root.at("grounded").get(def.grounded);
    root.at("hidden").get(def.hidden);
    root.at("draw-group").get(def.drawGroup);
    root.at("picking-item").get(def.pickingItem);
    root.at("script-name").get(def.scriptName);
    root.at("ui-layout").get(def.uiLayout);
    root.at("inventory-size").get(def.inventorySize);
    root.at("tick-interval").get(def.tickInterval);

    if (root.has("fields")) {
        def.dataStruct = std::make_unique<StructLayout>();
        def.dataStruct->deserialize(root["fields"]);

        perform_user_block_fields(def.name, *def.dataStruct);
    }

    if (def.tickInterval == 0) {
        def.tickInterval = 1;
    }

    if (def.hidden && def.pickingItem == def.name + BLOCK_ITEM_SUFFIX) {
        def.pickingItem = CORE_EMPTY;
    }
}

void ContentLoader::loadCustomBlockModel(Block& def, const dv::value& primitives) {
    if (primitives.has("aabbs")) {
        const auto& modelboxes = primitives["aabbs"];
        for (uint i = 0; i < modelboxes.size(); i++) {
            // Parse aabb
            const auto& boxarr = modelboxes[i];
            AABB modelbox;
            modelbox.a = glm::vec3(
                boxarr[0].asNumber(), boxarr[1].asNumber(), boxarr[2].asNumber()
            );
            modelbox.b = glm::vec3(
                boxarr[3].asNumber(), boxarr[4].asNumber(), boxarr[5].asNumber()
            );
            modelbox.b += modelbox.a;
            def.modelBoxes.push_back(modelbox);

            if (boxarr.size() == 7) {
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.emplace_back(boxarr[6].asString());
                }
            } else if (boxarr.size() == 12) {
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.emplace_back(boxarr[j].asString());
                }
            } else {
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.emplace_back("notfound");
                }
            }
        }
    }
    if (primitives.has("tetragons")) {
        const auto& modeltetragons = primitives["tetragons"];
        for (uint i = 0; i < modeltetragons.size(); i++) {
            // Parse tetragon to points
            const auto& tgonobj = modeltetragons[i];
            glm::vec3 p1(
                tgonobj[0].asNumber(), tgonobj[1].asNumber(), tgonobj[2].asNumber()
            );
            glm::vec3 xw(
                tgonobj[3].asNumber(), tgonobj[4].asNumber(), tgonobj[5].asNumber()
            );
            glm::vec3 yh(
                tgonobj[6].asNumber(), tgonobj[7].asNumber(), tgonobj[8].asNumber()
            );
            def.modelExtraPoints.push_back(p1);
            def.modelExtraPoints.push_back(p1 + xw);
            def.modelExtraPoints.push_back(p1 + xw + yh);
            def.modelExtraPoints.push_back(p1 + yh);

            def.modelTextures.emplace_back(tgonobj[9].asString());
        }
    }
}

void ContentLoader::loadItem(
    ItemDef& def, const std::string& name, const fs::path& file
) {
    auto root = files::read_json(file);

    if (root.has("parent")) {
        const auto& parentName = root["parent"].asString();
        auto parentDef = this->builder.items.get(parentName);
        if (parentDef == nullptr) {
            throw std::runtime_error(
                "Failed to find parent(" + parentName + ") for " + name
            );
        }
        parentDef->cloneTo(def);
    }
    root.at("caption").get(def.caption);

    std::string iconTypeStr = "";
    root.at("icon-type").get(iconTypeStr);
    if (iconTypeStr == "none") {
        def.iconType = item_icon_type::none;
    } else if (iconTypeStr == "block") {
        def.iconType = item_icon_type::block;
    } else if (iconTypeStr == "sprite") {
        def.iconType = item_icon_type::sprite;
    } else if (iconTypeStr.length()) {
        logger.error() << name << ": unknown icon type" << iconTypeStr;
    }
    root.at("icon").get(def.icon);
    root.at("placing-block").get(def.placingBlock);
    root.at("script-name").get(def.scriptName);
    root.at("stack-size").get(def.stackSize);

    // item light emission [r, g, b] where r,g,b in range [0..15]
    if (auto found = root.at("emission")) {
        const auto& emissionarr = *found;
        def.emission[0] = emissionarr[0].asNumber();
        def.emission[1] = emissionarr[1].asNumber();
        def.emission[2] = emissionarr[2].asNumber();
    }
}

void ContentLoader::loadEntity(
    EntityDef& def, const std::string& name, const fs::path& file
) {
    auto root = files::read_json(file);

    if (root.has("parent")) {
        const auto& parentName = root["parent"].asString();
        auto parentDef = this->builder.entities.get(parentName);
        if (parentDef == nullptr) {
            throw std::runtime_error(
                "Failed to find parent(" + parentName + ") for " + name
            );
        }
        parentDef->cloneTo(def);
    }

    if (auto found = root.at("components")) {
        for (const auto& elem : *found) {
            def.components.emplace_back(elem.asString());
        }
    }
    if (auto found = root.at("hitbox")) {
        const auto& arr = *found;
        def.hitbox = glm::vec3(
            arr[0].asNumber(), arr[1].asNumber(), arr[2].asNumber()
        );
    }
    if (auto found = root.at("sensors")) {
        const auto& arr = *found;
        for (size_t i = 0; i < arr.size(); i++) {
            const auto& sensorarr = arr[i];
            const auto& sensorType = sensorarr[0].asString();
            if (sensorType == "aabb") {
                def.boxSensors.emplace_back(
                    i,
                    AABB {
                        {sensorarr[1].asNumber(),
                            sensorarr[2].asNumber(),
                            sensorarr[3].asNumber()},
                        {sensorarr[4].asNumber(),
                            sensorarr[5].asNumber(),
                            sensorarr[6].asNumber()}
                    }
                );
            } else if (sensorType == "radius") {
                def.radialSensors.emplace_back(i, sensorarr[1].asNumber());
            } else {
                logger.error()
                    << name << ": sensor #" << i << " - unknown type "
                    << util::quote(sensorType);
            }
        }
    }
    root.at("save").get(def.save.enabled);
    root.at("save-skeleton-pose").get(def.save.skeleton.pose);
    root.at("save-skeleton-textures").get(def.save.skeleton.textures);
    root.at("save-body-velocity").get(def.save.body.velocity);
    root.at("save-body-settings").get(def.save.body.settings);

    std::string bodyTypeName;
    root.at("body-type").get(bodyTypeName);
    if (auto bodyType = BodyType_from(bodyTypeName)) {
        def.bodyType = *bodyType;
    }

    root.at("skeleton-name").get(def.skeletonName);
    root.at("blocking").get(def.blocking);
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
    root.at("steps-sound").get(def.stepsSound);
    root.at("place-sound").get(def.placeSound);
    root.at("break-sound").get(def.breakSound);
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
    std::vector<std::pair<std::string, std::string>> pendingDefs;
    auto getJsonParent = [this](const std::string& prefix, const std::string& name) {
            auto configFile = pack->folder / fs::path(prefix + "/" + name + ".json");
            std::string parent;
            if (fs::exists(configFile)) {
                auto root = files::read_json(configFile);
                root.at("parent").get(parent);
            }
            return parent;
        };
    auto processName = [this](const std::string& name) {
        auto colon = name.find(':');
        auto new_name = name;
        std::string full =
            colon == std::string::npos ? pack->id + ":" + name : name;
        if (colon != std::string::npos) new_name[colon] = '/';

        return std::make_pair(full, new_name);
    };

    if (auto found = root.at("blocks")) {
        const auto& blocksarr = *found;
        for (size_t i = 0; i < blocksarr.size(); i++) {
            auto [full, name] = processName(blocksarr[i].asString());
            auto parent = getJsonParent("blocks", name);
            if (parent.empty() || builder.blocks.get(parent)) {
                // No dependency or dependency already loaded/exists in another
                // content pack
                auto& def = builder.blocks.create(full);
                loadBlock(def, full, name);
                stats->totalBlocks++;
            } else {
                // Dependency not loaded yet, add to pending items
                pendingDefs.emplace_back(full, name);
            }
        }

        // Resolve dependencies for pending items
        bool progressMade = true;
        while (!pendingDefs.empty() && progressMade) {
            progressMade = false;

            for (auto it = pendingDefs.begin(); it != pendingDefs.end();) {
                auto parent = getJsonParent("blocks", it->second);
                if (builder.blocks.get(parent)) {
                    // Dependency resolved or parent exists in another pack,
                    // load the item
                    auto& def = builder.blocks.create(it->first);
                    loadBlock(def, it->first, it->second);
                    stats->totalBlocks++;
                    it = pendingDefs.erase(it);  // Remove resolved item
                    progressMade = true;
                } else {
                    ++it;
                }
            }
        }

        if (!pendingDefs.empty()) {
            // Handle circular dependencies or missing dependencies
            // You can log an error or throw an exception here if necessary
            throw std::runtime_error("Unresolved block dependencies detected.");
        }
    }

    if (auto found = root.at("items")) {
        const auto& itemsarr = *found;
        for (size_t i = 0; i < itemsarr.size(); i++) {
            auto [full, name] = processName(itemsarr[i].asString());
            auto parent = getJsonParent("items", name);
            if (parent.empty() || builder.items.get(parent)) {
                // No dependency or dependency already loaded/exists in another
                // content pack
                auto& def = builder.items.create(full);
                loadItem(def, full, name);
                stats->totalItems++;
            } else {
                // Dependency not loaded yet, add to pending items
                pendingDefs.emplace_back(full, name);
            }
        }

        // Resolve dependencies for pending items
        bool progressMade = true;
        while (!pendingDefs.empty() && progressMade) {
            progressMade = false;

            for (auto it = pendingDefs.begin(); it != pendingDefs.end();) {
                auto parent = getJsonParent("items", it->second);
                if (builder.items.get(parent)) {
                    // Dependency resolved or parent exists in another pack,
                    // load the item
                    auto& def = builder.items.create(it->first);
                    loadItem(def, it->first, it->second);
                    stats->totalItems++;
                    it = pendingDefs.erase(it);  // Remove resolved item
                    progressMade = true;
                } else {
                    ++it;
                }
            }
        }

        if (!pendingDefs.empty()) {
            // Handle circular dependencies or missing dependencies
            // You can log an error or throw an exception here if necessary
            throw std::runtime_error("Unresolved item dependencies detected.");
        }
    }

    if (auto found = root.at("entities")) {
        const auto& entitiesarr = *found;
        for (size_t i = 0; i < entitiesarr.size(); i++) {
            auto [full, name] = processName(entitiesarr[i].asString());
            auto parent = getJsonParent("entities", name);
            if (parent.empty() || builder.entities.get(parent)) {
                // No dependency or dependency already loaded/exists in another
                // content pack
                auto& def = builder.entities.create(full);
                loadEntity(def, full, name);
                stats->totalEntities++;
            } else {
                // Dependency not loaded yet, add to pending items
                pendingDefs.emplace_back(full, name);
            }
        }

        // Resolve dependencies for pending items
        bool progressMade = true;
        while (!pendingDefs.empty() && progressMade) {
            progressMade = false;

            for (auto it = pendingDefs.begin(); it != pendingDefs.end();) {
                auto parent = getJsonParent("entities", it->second);
                if (builder.entities.get(parent)) {
                    // Dependency resolved or parent exists in another pack,
                    // load the item
                    auto& def = builder.entities.create(it->first);
                    loadEntity(def, it->first, it->second);
                    stats->totalEntities++;
                    it = pendingDefs.erase(it);  // Remove resolved item
                    progressMade = true;
                } else {
                    ++it;
                }
            }
        }

        if (!pendingDefs.empty()) {
            // Handle circular dependencies or missing dependencies
            // You can log an error or throw an exception here if necessary
            throw std::runtime_error(
                "Unresolved entities dependencies detected."
            );
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
        for (const auto& [key, arr] : resRoot.asObject()) {
            if (auto resType = ResourceType_from(key)) {
                loadResources(*resType, arr);
            } else {
                logger.warning() << "unknown resource type: " << key;
            }
        }
    }
}

void ContentLoader::loadResources(ResourceType type, const dv::value& list) {
    for (size_t i = 0; i < list.size(); i++) {
        builder.resourceIndices[static_cast<size_t>(type)].add(
            pack->id + ":" + list[i].asString(), nullptr
        );
    }
}
