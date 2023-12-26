#include "ContentLoader.h"

#include <iostream>
#include <string>
#include <memory>
#include <glm/glm.hpp>

#include "Content.h"
#include "../voxels/Block.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../typedefs.h"

#include "ContentPack.h"
#include "../logic/scripting/scripting.h"

namespace fs = std::filesystem;

ContentLoader::ContentLoader(ContentPack* pack) : pack(pack) {
}

// TODO: add basic validation and logging
Block* ContentLoader::loadBlock(std::string name, fs::path file) {
    std::unique_ptr<json::JObject> root(files::read_json(file));
    std::unique_ptr<Block> def(new Block(name));

    // block texturing
    if (root->has("texture")) {
        std::string texture;
        root->str("texture", texture);
        for (uint i = 0; i < 6; i++)
            def->textureFaces[i] = texture;
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
    root->flag("sky-light-passing", def->skyLightPassing);
    root->num("draw-group", def->drawGroup);

    return def.release();
}

void ContentLoader::load(ContentBuilder* builder) {
    std::cout << "-- loading pack [" << pack->id << "]" << std::endl;

    auto folder = pack->folder;
    auto root = files::read_json(pack->getContentFile());

    json::JArray* blocksarr = root->arr("blocks");
    if (blocksarr) {
        for (uint i = 0; i < blocksarr->size(); i++) {
            std::string name = blocksarr->str(i); 
            std::string prefix = pack->id+":"+name;
            fs::path blockfile = folder/fs::path("blocks/"+name+".json");
            Block* block = loadBlock(prefix, blockfile);
            builder->add(block);
            fs::path scriptfile = folder/fs::path("scripts/"+name+".lua");
            if (fs::is_regular_file(scriptfile)) {
                scripting::load_block_script(prefix, scriptfile, &block->rt.funcsset);
            }
        }
    }
}
