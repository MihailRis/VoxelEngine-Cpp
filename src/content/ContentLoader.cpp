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
#include "presets/ParticlesPreset.hpp"

namespace fs = std::filesystem;
using namespace data;

static debug::Logger logger("content-loader");

ContentLoader::ContentLoader(
    ContentPack* pack, ContentBuilder& builder, const ResPaths& paths
)
    : pack(pack), builder(builder), paths(paths) {
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
            if (fs::is_regular_file(file) && files::is_data_file(file)) {
                auto map = files::read_object(file);
                std::string id = prefix.empty() ? name : prefix + ":" + name;
                detected.emplace_back(id);
            } else if (fs::is_directory(file) && 
                       file.extension() != fs::u8path(".files")) {
                detect_defs(file, name, detected);
            }
        }
    }
}

static void detect_defs_pairs(
    const fs::path& folder,
    const std::string& prefix,
    std::vector<std::tuple<std::string, std::string>>& detected
) {
    if (fs::is_directory(folder)) {
        for (const auto& entry : fs::directory_iterator(folder)) {
            const fs::path& file = entry.path();
            std::string name = file.stem().string();
            if (name[0] == '_') {
                continue;
            }
            if (fs::is_regular_file(file) && files::is_data_file(file)) {
                try {
                    auto map = files::read_object(file);
                    auto id = prefix.empty() ? name : prefix + ":" + name;
                    auto caption = util::id_to_caption(id);
                    map.at("caption").get(caption);
                    detected.emplace_back(id, name);
                } catch (const std::runtime_error& err) {
                    logger.error() << err.what();
                }
            } else if (fs::is_directory(file) && 
                       file.extension() != fs::u8path(".files")) {
                detect_defs_pairs(file, name, detected);
            }
        }
    }
}

std::vector<std::tuple<std::string, std::string>> ContentLoader::scanContent(
    const ContentPack& pack, ContentType type
) {
    std::vector<std::tuple<std::string, std::string>> detected;
    detect_defs_pairs(
        pack.folder / ContentPack::getFolderFor(type), pack.id, detected);
    return detected;
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
    auto contentFile = pack->getContentFile();
    auto blocksFolder = folder / ContentPack::BLOCKS_FOLDER;
    auto itemsFolder = folder / ContentPack::ITEMS_FOLDER;
    auto entitiesFolder = folder / ContentPack::ENTITIES_FOLDER;

    dv::value root;
    if (fs::is_regular_file(contentFile)) {
        root = files::read_json(contentFile);
    } else {
        root = dv::object();
    }

    bool modified = false;
    modified |= fixPackIndices(blocksFolder, root, "blocks");
    modified |= fixPackIndices(itemsFolder, root, "items");
    modified |= fixPackIndices(entitiesFolder, root, "entities");

    if (modified) {
        // rewrite modified json
        files::write_json(contentFile, root);
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
    def.properties = root;

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
    std::string modelTypeName = to_string(def.model);
    root.at("model").get(modelTypeName);
    root.at("model-name").get(def.modelName);
    if (auto model = BlockModel_from(modelTypeName)) {
        if (*model == BlockModel::custom && def.customModelRaw == nullptr) {
            if (root.has("model-primitives")) {
                def.customModelRaw = root["model-primitives"];
            } else if (def.modelName.empty()) {
                throw std::runtime_error(name + ": no 'model-primitives' or 'model-name' found");
            }
            for (uint i = 0; i < 6; i++) {
                std::string& texture = def.textureFaces[i];
                if (texture == TEXTURE_NOTFOUND) {
                    texture = "";
                }
            }
        }
        def.model = *model;
    } else if (!modelTypeName.empty()) {
        logger.error() << "unknown model: " << modelTypeName;
        def.model = BlockModel::none;
    }

    std::string cullingModeName = to_string(def.culling);
    root.at("culling").get(cullingModeName);
    if (auto mode = CullingMode_from(cullingModeName)) {
        def.culling = *mode;
    } else {
        logger.error() << "unknown culling mode: " << cullingModeName;
    }

    root.at("material").get(def.material);

    // rotation profile
    std::string profile = def.rotations.name;
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
    }

    // block light emission [r, g, b] where r,g,b in range [0..15]
    if (auto found = root.at("emission")) {
        const auto& emissionarr = *found;
        for (size_t i = 0; i < 3; i++) {
            def.emission[i] = std::clamp(emissionarr[i].asInteger(), static_cast<integer_t>(0), static_cast<integer_t>(15));
        }
    }

    // block size
    if (auto found = root.at("size")) {
        const auto& sizearr = *found;
        def.size.x = sizearr[0].asInteger();
        def.size.y = sizearr[1].asInteger();
        def.size.z = sizearr[2].asInteger();
        if (def.size.x < 1 || def.size.y < 1 || def.size.z < 1) {
            throw std::runtime_error(
                "block " + util::quote(def.name) + ": invalid block size"
            );
        }
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
    root.at("surface-replacement").get(def.surfaceReplacement);
    root.at("script-name").get(def.scriptName);
    root.at("ui-layout").get(def.uiLayout);
    root.at("inventory-size").get(def.inventorySize);
    root.at("tick-interval").get(def.tickInterval);
    root.at("overlay-texture").get(def.overlayTexture);
    root.at("translucent").get(def.translucent);

    if (root.has("fields")) {
        def.dataStruct = std::make_unique<StructLayout>();
        def.dataStruct->deserialize(root["fields"]);

        perform_user_block_fields(def.name, *def.dataStruct);
    }

    if (root.has("particles")) {
        def.particles = std::make_unique<ParticlesPreset>();
        def.particles->deserialize(root["particles"]);
    }

    if (def.tickInterval == 0) {
        def.tickInterval = 1;
    }

    if (def.hidden && def.pickingItem == def.name + BLOCK_ITEM_SUFFIX) {
        def.pickingItem = CORE_EMPTY;
    }
}

void ContentLoader::loadItem(
    ItemDef& def, const std::string& name, const fs::path& file
) {
    auto root = files::read_json(file);
    def.properties = root;

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
        def.iconType = ItemIconType::NONE;
    } else if (iconTypeStr == "block") {
        def.iconType = ItemIconType::BLOCK;
    } else if (iconTypeStr == "sprite") {
        def.iconType = ItemIconType::SPRITE;
    } else if (iconTypeStr.length()) {
        logger.error() << name << ": unknown icon type" << iconTypeStr;
    }
    root.at("icon").get(def.icon);
    root.at("placing-block").get(def.placingBlock);
    root.at("script-name").get(def.scriptName);
    root.at("model-name").get(def.modelName);
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

    if (!def.hidden) {
        auto& item = builder.items.create(full + BLOCK_ITEM_SUFFIX);
        item.generated = true;
        item.caption = def.caption;
        item.iconType = ItemIconType::BLOCK;
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
}

static std::tuple<std::string, std::string, std::string> create_unit_id(
    const std::string& packid, const std::string& name
) {
    size_t colon = name.find(':');
    if (colon == std::string::npos) {
        return {packid, packid + ":" + name, name};
    }
    auto otherPackid = name.substr(0, colon);
    auto full = otherPackid + ":" + name;
    return {otherPackid, full, otherPackid + "/" + name};
}

void ContentLoader::loadBlockMaterial(
    BlockMaterial& def, const fs::path& file
) {
    auto root = files::read_json(file);
    root.at("steps-sound").get(def.stepsSound);
    root.at("place-sound").get(def.placeSound);
    root.at("break-sound").get(def.breakSound);
}

void ContentLoader::loadContent(const dv::value& root) {
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
}

static inline void foreach_file(
    const fs::path& dir, std::function<void(const fs::path&)> handler
) {
    if (fs::is_directory(dir)) {
        for (const auto& entry : fs::directory_iterator(dir)) {
            const auto& path = entry.path();
            if (fs::is_directory(path)) {
                continue;
            }
            handler(path);
        }
    }
}

void ContentLoader::load() {
    logger.info() << "loading pack [" << pack->id << "]";

    fixPackIndices();

    auto folder = pack->folder;

    // Load world generators
    fs::path generatorsDir = folder / fs::u8path("generators");
    foreach_file(generatorsDir, [this](const fs::path& file) {
        std::string name = file.stem().u8string();
        auto [packid, full, filename] =
            create_unit_id(pack->id, file.stem().u8string());

        auto& def = builder.generators.create(full);
        try {
            loadGenerator(def, full, name);
        } catch (const std::runtime_error& err) {
            throw std::runtime_error("generator '"+full+"': "+err.what());
        }
    });

    // Load pack resources.json
    fs::path resourcesFile = folder / fs::u8path("resources.json");
    if (fs::exists(resourcesFile)) {
        auto resRoot = files::read_json(resourcesFile);
        for (const auto& [key, arr] : resRoot.asObject()) {
            if (auto resType = ResourceType_from(key)) {
                loadResources(*resType, arr);
            } else {
                // Ignore unknown resources
                logger.warning() << "unknown resource type: " << key;
            }
        }
    }

    // Load pack resources aliases
    fs::path aliasesFile = folder / fs::u8path("resource-aliases.json");
    if (fs::exists(aliasesFile)) {
        auto resRoot = files::read_json(aliasesFile);
        for (const auto& [key, arr] : resRoot.asObject()) {
            if (auto resType = ResourceType_from(key)) {
                loadResourceAliases(*resType, arr);
            } else {
                // Ignore unknown resources
                logger.warning() << "unknown resource type: " << key;
            }
        }
    }

    // Load block materials
    fs::path materialsDir = folder / fs::u8path("block_materials");    
    if (fs::is_directory(materialsDir)) {
        for (const auto& entry : fs::directory_iterator(materialsDir)) {
            const auto& file = entry.path();
            auto [packid, full, filename] =
                create_unit_id(pack->id, file.stem().u8string());
            loadBlockMaterial(
                builder.createBlockMaterial(full),
                materialsDir / fs::u8path(filename + ".json")
            );
        }
    }

    // Load skeletons
    fs::path skeletonsDir = folder / fs::u8path("skeletons");
    foreach_file(skeletonsDir, [this](const fs::path& file) {
        std::string name = pack->id + ":" + file.stem().u8string();
        std::string text = files::read_string(file);
        builder.add(
            rigging::SkeletonConfig::parse(text, file.u8string(), name)
        );
    });

    // Process content.json and load defined content units
    auto contentFile = pack->getContentFile();
    if (fs::exists(contentFile)) {
        loadContent(files::read_json(contentFile));
    }
}

template <class T>
static void load_scripts(Content& content, ContentUnitDefs<T>& units) {
    for (const auto& [name, def] : units.getDefs()) {
        size_t pos = name.find(':');
        if (pos == std::string::npos) {
            throw std::runtime_error("invalid content unit name");
        }
        const auto runtime = content.getPackRuntime(name.substr(0, pos));
        const auto& pack = runtime->getInfo();
        const auto& folder = pack.folder;
        auto scriptfile = folder / fs::path("scripts/" + def->scriptName + ".lua");
        if (fs::is_regular_file(scriptfile)) {
            scripting::load_content_script(
                runtime->getEnvironment(),
                name,
                scriptfile,
                pack.id + ":scripts/" + def->scriptName + ".lua",
                def->rt.funcsset
            );
        }
    }
}

void ContentLoader::loadScripts(Content& content) {
    load_scripts(content, content.blocks);
    load_scripts(content, content.items);

    for (const auto& [packid, runtime] : content.getPacks()) {
        const auto& pack = runtime->getInfo();
        const auto& folder = pack.folder;
        
        // Load main world script
        fs::path scriptFile = folder / fs::path("scripts/world.lua");
        if (fs::is_regular_file(scriptFile)) {
            scripting::load_world_script(
                runtime->getEnvironment(),
                pack.id,
                scriptFile,
                pack.id + ":scripts/world.lua",
                runtime->worldfuncsset
            );
        }
        // Load entity components
        fs::path componentsDir = folder / fs::u8path("scripts/components");
        foreach_file(componentsDir, [&pack](const fs::path& file) {
            auto name = pack.id + ":" + file.stem().u8string();
            scripting::load_entity_component(
                name,
                file,
                pack.id + ":scripts/components/" + file.filename().u8string()
            );
        });
    }
}

void ContentLoader::loadResources(ResourceType type, const dv::value& list) {
    for (size_t i = 0; i < list.size(); i++) {
        builder.resourceIndices[static_cast<size_t>(type)].add(
            pack->id + ":" + list[i].asString(), nullptr
        );
    }
}

void ContentLoader::loadResourceAliases(ResourceType type, const dv::value& aliases) {
    for (const auto& [alias, name] : aliases.asObject()) {
        builder.resourceIndices[static_cast<size_t>(type)].addAlias(
            name.asString(), alias
        );
    }
}
