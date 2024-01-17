#include "ContentLoader.h"

#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <glm/glm.hpp>

#include "Content.h"
#include "../items/ItemDef.h"
#include "../util/listutil.h"
#include "../voxels/Block.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../typedefs.h"

#include "ContentPack.h"
#include "../logic/scripting/scripting.h"

namespace fs = std::filesystem;

ContentLoader::ContentLoader(ContentPack* pack) : pack(pack) {
}

bool ContentLoader::fixPackIndices(fs::path folder, 
                                   json::JObject* indicesRoot,
                                   std::string contentSection) {

    std::vector<std::string> detected;
    std::vector<std::string> indexed;
    if (fs::is_directory(folder)) {
        for (auto entry : fs::directory_iterator(folder)) {
            fs::path file = entry.path();
            if (fs::is_regular_file(file) && file.extension() == ".json") {
                std::string name = file.stem().string();
                if (name[0] == '_')
                    continue;
                detected.push_back(name);
            }
        }
    }

    bool modified = false;
    if (!indicesRoot->has(contentSection)) {
        indicesRoot->putArray(contentSection);
    }
    json::JArray* arr = indicesRoot->arr(contentSection);
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

    std::unique_ptr<json::JObject> root;
    if (fs::is_regular_file(indexFile)) {
        root.reset(files::read_json(indexFile));
    } else {
        root.reset(new json::JObject());
    }

    bool modified = false;

    modified |= fixPackIndices(blocksFolder, root.get(), "blocks");
    modified |= fixPackIndices(itemsFolder, root.get(), "items");

    if (modified){
        // rewrite modified json
        std::cout << indexFile << std::endl;
        files::write_json(indexFile, root.get());
    }
}

// TODO: add basic validation and logging
void ContentLoader::loadBlock(Block* def, std::string name, fs::path file) {
    std::unique_ptr<json::JObject> root(files::read_json(file));

    // block texturing
    if (root->has("texture")) {
        std::string texture;
        root->str("texture", texture);
        for (uint i = 0; i < 6; i++) {
            def->textureFaces[i] = texture;
        }
    } else if (root->has("texture-faces")) {
        json::JArray* texarr = root->arr("texture-faces");
        for (uint i = 0; i < 6; i++) {
            def->textureFaces[i] = texarr->str(i);
        }
    }

    // block model
    std::string model = "block";
    root->str("model", model);
    if (model == "block") def->model = BlockModel::block;
    else if (model == "aabb") def->model = BlockModel::aabb;
    else if (model == "custom") { 
        def->model = BlockModel::custom;
        if (root->has("model-primitives")) {
            loadCustomBlockModel(def, root->obj("model-primitives"));
        }
        else {
            std::cerr << "ERROR occured while block "
                       << name << " parsed: no \"model-primitives\" found" << std::endl;
        }
    }
    else if (model == "X") def->model = BlockModel::xsprite;
    else if (model == "none") def->model = BlockModel::none;
    else {
        std::cerr << "unknown model " << model << std::endl;
        def->model = BlockModel::none;
    }

    // rotation profile
    std::string profile = "none";
    root->str("rotation", profile);
    def->rotatable = profile != "none";
    if (profile == "pipe") {
        def->rotations = BlockRotProfile::PIPE;
    } else if (profile == "pane") {
        def->rotations = BlockRotProfile::PANE;
    } else if (profile != "none") {
        std::cerr << "unknown rotation profile " << profile << std::endl;
        def->rotatable = false;
    }
    
    // block hitbox AABB [x, y, z, width, height, depth]
    json::JArray* boxobj = root->arr("hitbox");
    if (boxobj) {
        AABB& aabb = def->hitbox;
        aabb.a = glm::vec3(boxobj->num(0), boxobj->num(1), boxobj->num(2));
        aabb.b = glm::vec3(boxobj->num(3), boxobj->num(4), boxobj->num(5));
        aabb.b += aabb.a;
    }

    // block light emission [r, g, b] where r,g,b in range [0..15]
    json::JArray* emissionobj = root->arr("emission");
    if (emissionobj) {
        def->emission[0] = emissionobj->num(0);
        def->emission[1] = emissionobj->num(1);
        def->emission[2] = emissionobj->num(2);
    }

    // primitive properties
    root->flag("obstacle", def->obstacle);
    root->flag("replaceable", def->replaceable);
    root->flag("light-passing", def->lightPassing);
    root->flag("breakable", def->breakable);
    root->flag("selectable", def->selectable);
    root->flag("grounded", def->grounded);
    root->flag("hidden", def->hidden);
    root->flag("sky-light-passing", def->skyLightPassing);
    root->num("draw-group", def->drawGroup);

    root->str("picking-item", def->pickingItem);
}

void ContentLoader::loadCustomBlockModel(Block* def, json::JObject* primitives) {
    if (primitives->has("aabbs")) {
        json::JArray* modelboxes = primitives->arr("aabbs");
        for (uint i = 0; i < modelboxes->size(); i++ ) {
            /* Parse aabb */
            json::JArray* boxobj = modelboxes->arr(i);
            AABB modelbox;
            modelbox.a = glm::vec3(boxobj->num(0), boxobj->num(1), boxobj->num(2));
            modelbox.b = glm::vec3(boxobj->num(3), boxobj->num(4), boxobj->num(5));
            modelbox.b += modelbox.a;
            def->modelBoxes.push_back(modelbox);

            if (boxobj->size() == 7)
                for (uint i = 6; i < 12; i++) {
                    def->modelTextures.push_back(boxobj->str(6));
                }
            else if (boxobj->size() == 12)
                for (uint i = 6; i < 12; i++) {
                    def->modelTextures.push_back(boxobj->str(i));
                }
            else
                for (uint i = 6; i < 12; i++) {
                    def->modelTextures.push_back("notfound");
                }
        }
    }
    if (primitives->has("tetragons")) {
        json::JArray* modeltetragons = primitives->arr("tetragons");
        for (uint i = 0; i < modeltetragons->size(); i++) {
            /* Parse tetragon to points */
            json::JArray* tgonobj = modeltetragons->arr(i);
            glm::vec3 p1(tgonobj->num(0), tgonobj->num(1), tgonobj->num(2)),
                    xw(tgonobj->num(3), tgonobj->num(4), tgonobj->num(5)),
                    yh(tgonobj->num(6), tgonobj->num(7), tgonobj->num(8));
            def->modelExtraPoints.push_back(p1);
            def->modelExtraPoints.push_back(p1+xw);
            def->modelExtraPoints.push_back(p1+xw+yh);
            def->modelExtraPoints.push_back(p1+yh);

            def->modelTextures.push_back(tgonobj->str(9));
        }
    }
}

void ContentLoader::loadItem(ItemDef* def, std::string name, fs::path file) {
    std::unique_ptr<json::JObject> root(files::read_json(file));
    std::string iconTypeStr = "none";
    root->str("icon-type", iconTypeStr);
    if (iconTypeStr == "none") {
        def->iconType = item_icon_type::none;
    } else if (iconTypeStr == "block") {
        def->iconType = item_icon_type::block;
    } else if (iconTypeStr == "sprite") {
        def->iconType = item_icon_type::sprite;
    } else {
        std::cerr << "unknown icon type" << iconTypeStr << std::endl;
    }
    root->str("icon", def->icon);
    root->str("placing-block", def->placingBlock);

    // item light emission [r, g, b] where r,g,b in range [0..15]
    json::JArray* emissionobj = root->arr("emission");
    if (emissionobj) {
        def->emission[0] = emissionobj->num(0);
        def->emission[1] = emissionobj->num(1);
        def->emission[2] = emissionobj->num(2);
    }
}

void ContentLoader::loadBlock(Block* def, std::string full, std::string name) {
    auto folder = pack->folder;

    fs::path configFile = folder/fs::path("blocks/"+name+".json");
    fs::path scriptfile = folder/fs::path("scripts/"+name+".lua");
    loadBlock(def, full, configFile);
    if (fs::is_regular_file(scriptfile)) {
        scripting::load_block_script(full, scriptfile, &def->rt.funcsset);
    }
}

void ContentLoader::loadItem(ItemDef* def, std::string full, std::string name) {
    auto folder = pack->folder;

    fs::path configFile = folder/fs::path("items/"+name+".json");
    fs::path scriptfile = folder/fs::path("scripts/"+name+".lua");
    loadItem(def, full, configFile);
    if (fs::is_regular_file(scriptfile)) {
        scripting::load_item_script(full, scriptfile, &def->rt.funcsset);
    }
}

void ContentLoader::load(ContentBuilder* builder) {
    std::cout << "-- loading pack [" << pack->id << "]" << std::endl;

    fixPackIndices();

    auto folder = pack->folder;
    if (!fs::is_regular_file(pack->getContentFile()))
        return;
    std::unique_ptr<json::JObject> root (files::read_json(pack->getContentFile()));

    json::JArray* blocksarr = root->arr("blocks");
    if (blocksarr) {
        for (uint i = 0; i < blocksarr->size(); i++) {
            std::string name = blocksarr->str(i);
            std::string full = pack->id+":"+name;
            auto def = builder->createBlock(full);
            loadBlock(def, full, name);
            if (!def->hidden) {
                auto item = builder->createItem(full+BLOCK_ITEM_SUFFIX);
                item->generated = true;
                item->iconType = item_icon_type::block;
                item->icon = full;
                item->placingBlock = full;
                
                for (uint j = 0; j < 4; j++) {
                    item->emission[j] = def->emission[j];
                }
            }
        }
    }

    json::JArray* itemsarr = root->arr("items");
    if (itemsarr) {
        for (uint i = 0; i < itemsarr->size(); i++) {
            std::string name = itemsarr->str(i);
            std::string full = pack->id+":"+name;
            loadItem(builder->createItem(full), full, name);
        }
    }
}
