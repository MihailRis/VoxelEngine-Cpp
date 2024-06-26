#include "ContentLoader.hpp"

#include "Content.hpp"
#include "ContentPack.hpp"
#include "ContentBuilder.hpp"
#include "../coders/json.hpp"
#include "../core_defs.hpp"
#include "../data/dynamic.hpp"
#include "../debug/Logger.hpp"
#include "../files/files.hpp"
#include "../items/ItemDef.hpp"
#include "../logic/scripting/scripting.hpp"
#include "../typedefs.hpp"
#include "../util/listutil.hpp"
#include "../voxels/Block.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <glm/glm.hpp>

namespace fs = std::filesystem;

static debug::Logger logger("content-loader");

ContentLoader::ContentLoader(ContentPack* pack) : pack(pack) {
}

bool ContentLoader::fixPackIndices(
    const fs::path& folder,
    dynamic::Map* indicesRoot,
    const std::string& contentSection
) {
    std::vector<std::string> detected;
    std::vector<std::string> indexed;
    if (fs::is_directory(folder)) {
        for (const auto& entry : fs::directory_iterator(folder)) {
            const fs::path& file = entry.path();
            if (fs::is_regular_file(file) && file.extension() == ".json") {
                std::string name = file.stem().string();
                if (name[0] == '_')
                    continue;
                detected.push_back(name);
            } else if (fs::is_directory(file)) {
                std::string space = file.stem().string();
                if (space[0] == '_')
                    continue;
                for (const auto& entry : fs::directory_iterator(file)) {
                    const fs::path& file = entry.path();
                    if (fs::is_regular_file(file) && file.extension() == ".json") {
                        std::string name = file.stem().string();
                        if (name[0] == '_')
                            continue;
                        detected.push_back(space + ':' + name);
                    }
                }
            }
        }
    }

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
    auto blocksFolder = folder/ContentPack::BLOCKS_FOLDER;
    auto itemsFolder = folder/ContentPack::ITEMS_FOLDER;

    dynamic::Map_sptr root;
    if (fs::is_regular_file(indexFile)) {
        root = files::read_json(indexFile);
    } else {
        root = dynamic::create_map();
    }

    bool modified = false;

    modified |= fixPackIndices(blocksFolder, root.get(), "blocks");
    modified |= fixPackIndices(itemsFolder, root.get(), "items");

    if (modified){
        // rewrite modified json
        files::write_json(indexFile, root.get());
    }
}

void ContentLoader::loadBlock(Block& def, const std::string& name, const fs::path& file) {
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
    std::string model = "block";
    root->str("model", model);
    if (model == "block") def.model = BlockModel::block;
    else if (model == "aabb") def.model = BlockModel::aabb;
    else if (model == "custom") { 
        def.model = BlockModel::custom;
        if (root->has("model-primitives")) {
            loadCustomBlockModel(def, root->map("model-primitives"));
        } else {
            logger.error() << name << ": no 'model-primitives' found";
        }
    }
    else if (model == "X") def.model = BlockModel::xsprite;
    else if (model == "none") def.model = BlockModel::none;
    else {
        logger.error() << "unknown model " << model;
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
            def.hitboxes[i].a = glm::vec3(box->num(0), box->num(1), box->num(2));
            def.hitboxes[i].b = glm::vec3(box->num(3), box->num(4), box->num(5));
            def.hitboxes[i].b += def.hitboxes[i].a;
        }
    } else if (auto boxarr = root->list("hitbox")){
        AABB aabb;
        aabb.a = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
        aabb.b = glm::vec3(boxarr->num(3), boxarr->num(4), boxarr->num(5));
        aabb.b += aabb.a;
        def.hitboxes = { aabb };
    } else if (!def.modelBoxes.empty()) {
        def.hitboxes = def.modelBoxes;
    } else {
        def.hitboxes = { AABB() };
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

    if (def.hidden && def.pickingItem == def.name+BLOCK_ITEM_SUFFIX) {
        def.pickingItem = CORE_EMPTY;
    }
}

void ContentLoader::loadCustomBlockModel(Block& def, dynamic::Map* primitives) {
    if (primitives->has("aabbs")) {
        auto modelboxes = primitives->list("aabbs");
        for (uint i = 0; i < modelboxes->size(); i++ ) {
            /* Parse aabb */
            auto boxarr = modelboxes->list(i);
            AABB modelbox;
            modelbox.a = glm::vec3(boxarr->num(0), boxarr->num(1), boxarr->num(2));
            modelbox.b = glm::vec3(boxarr->num(3), boxarr->num(4), boxarr->num(5));
            modelbox.b += modelbox.a;
            def.modelBoxes.push_back(modelbox);

            if (boxarr->size() == 7)
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.push_back(boxarr->str(6));
                }
            else if (boxarr->size() == 12)
                for (uint j = 6; j < 12; j++) {
                    def.modelTextures.push_back(boxarr->str(j));
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
            def.modelExtraPoints.push_back(p1+xw);
            def.modelExtraPoints.push_back(p1+xw+yh);
            def.modelExtraPoints.push_back(p1+yh);

            def.modelTextures.push_back(tgonobj->str(9));
        }
    }
}

void ContentLoader::loadItem(ItemDef& def, const std::string& name, const fs::path& file) {
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
    } else if (iconTypeStr.length()){
        logger.error() << name << ": unknown icon type" << iconTypeStr;
    }
    root->str("icon", def.icon);
    root->str("placing-block", def.placingBlock);
    root->str("script-name", def.scriptName);
    root->num("stack-size", def.stackSize);

    // item light emission [r, g, b] where r,g,b in range [0..15]
    auto emissionarr = root->list("emission");
    if (emissionarr) {
        def.emission[0] = emissionarr->num(0);
        def.emission[1] = emissionarr->num(1);
        def.emission[2] = emissionarr->num(2);
    }
}

void ContentLoader::loadBlock(Block& def, const std::string& full, const std::string& name) {
    auto folder = pack->folder;

    fs::path configFile = folder/fs::path("blocks/"+name+".json");
    if (fs::exists(configFile)) loadBlock(def, full, configFile);

    fs::path scriptfile = folder/fs::path("scripts/"+def.scriptName+".lua");
    if (fs::is_regular_file(scriptfile)) {
        scripting::load_block_script(env, full, scriptfile, def.rt.funcsset);
    }
}

void ContentLoader::loadItem(ItemDef& def, const std::string& full, const std::string& name) {
    auto folder = pack->folder;

    fs::path configFile = folder/fs::path("items/"+name+".json");
    if (fs::exists(configFile)) loadItem(def, full, configFile);

    fs::path scriptfile = folder/fs::path("scripts/"+def.scriptName+".lua");
    if (fs::is_regular_file(scriptfile)) {
        scripting::load_item_script(env, full, scriptfile, def.rt.funcsset);
    }
}

void ContentLoader::loadBlockMaterial(BlockMaterial& def, const fs::path& file) {
    auto root = files::read_json(file);
    root->str("steps-sound", def.stepsSound);
    root->str("place-sound", def.placeSound);
    root->str("break-sound", def.breakSound);
}

void ContentLoader::load(ContentBuilder& builder) {
    logger.info() << "loading pack [" << pack->id << "]";

    auto runtime = std::make_unique<ContentPackRuntime>(
        *pack, scripting::create_pack_environment(*pack)
    );
    env = runtime->getEnvironment();
    ContentPackStats& stats = runtime->getStatsWriteable();
    builder.add(std::move(runtime));

    fixPackIndices();

    auto folder = pack->folder;

    fs::path scriptFile = folder/fs::path("scripts/world.lua");
    if (fs::is_regular_file(scriptFile)) {
        scripting::load_world_script(env, pack->id, scriptFile);
    }

    if (!fs::is_regular_file(pack->getContentFile()))
        return;

    auto root = files::read_json(pack->getContentFile());
    auto blocksarr = root->list("blocks");
    if (blocksarr) {
        for (uint i = 0; i < blocksarr->size(); i++) {
            std::string name = blocksarr->str(i);
            auto colon = name.find(':');
            std::string full = colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.blocks.create(full);
            if (colon != std::string::npos) {
                def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            }
            loadBlock(def, full, name);
            stats.totalBlocks++;
            if (!def.hidden) {
                auto& item = builder.items.create(full+BLOCK_ITEM_SUFFIX);
                item.generated = true;
                item.caption = def.caption;
                item.iconType = item_icon_type::block;
                item.icon = full;
                item.placingBlock = full;
                
                for (uint j = 0; j < 4; j++) {
                    item.emission[j] = def.emission[j];
                }
                stats.totalItems++;
            }
        }
    }

    auto itemsarr = root->list("items");
    if (itemsarr) {
        for (uint i = 0; i < itemsarr->size(); i++) {
            std::string name = itemsarr->str(i);
            auto colon = name.find(':');
            std::string full = colon == std::string::npos ? pack->id + ":" + name : name;
            if (colon != std::string::npos) name[colon] = '/';
            auto& def = builder.items.create(full);
            if (colon != std::string::npos) def.scriptName = name.substr(0, colon) + '/' + def.scriptName;
            loadItem(def, full, name);
            stats.totalItems++;
        }
    }

    fs::path materialsDir = folder / fs::u8path("block_materials");
    if (fs::is_directory(materialsDir)) {
        for (const auto& entry : fs::directory_iterator(materialsDir)) {
            const fs::path& file = entry.path();
            std::string name = pack->id+":"+file.stem().u8string();
            loadBlockMaterial(builder.createBlockMaterial(name), file);
        }
    }
}
