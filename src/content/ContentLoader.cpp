#include "ContentLoader.h"

#include <iostream>
#include <string>
#include <memory>

#include "Content.h"
#include "../voxels/Block.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../typedefs.h"

#include <glm/glm.hpp>

// don't ask
using glm::vec3;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::unique_ptr;
using std::filesystem::path;

ContentLoader::ContentLoader(path folder) : folder(folder) {}

// TODO: add basic validation and logging
Block* ContentLoader::loadBlock(string name, path file) {
    unique_ptr<json::JObject> root(files::read_json(file));
    unique_ptr<Block> def(new Block(name));

    // block texturing
    if (root->has("texture")) {
        string texture;
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
    string model = "block";
    root->str("model", model);
    if (model == "block") def->model = BlockModel::block;
    else if (model == "aabb") def->model = BlockModel::aabb;
    else if (model == "X") def->model = BlockModel::xsprite;
    else if (model == "none") def->model = BlockModel::none;
    else {
        cerr << "unknown model " << model << endl;
        def->model = BlockModel::none;
    }
    
    // block hitbox AABB [x, y, z, width, height, depth]
    json::JArray* hitboxobj = root->arr("hitbox");
    if (hitboxobj) {
        AABB& aabb = def->hitbox;
        aabb.a = vec3(hitboxobj->num(0), hitboxobj->num(1), hitboxobj->num(2));
        aabb.b = vec3(hitboxobj->num(3), hitboxobj->num(4), hitboxobj->num(5));
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
    root->flag("rotatable", def->rotatable);
    root->flag("sky-light-passing", def->skyLightPassing);
    root->num("draw-group", def->drawGroup);

    return def.release();
}

void ContentLoader::load(ContentBuilder* builder) {
    cout << "-- loading content " << folder << endl;

    path file = folder / path("package.json");
    string source = files::read_string(file);

    unique_ptr<json::JObject> root = nullptr;
    try {
        root.reset(json::parse(file.filename().string(), source));
    } catch (const parsing_error& error) {
        cerr << error.errorLog() << endl;
        throw std::runtime_error("could not load content package");
    }

    string id;
    string version;
    root->str("id", id);
    root->str("version", version);

    cout << "    id: " << id << endl;
    cout << "    version: " << version << endl;

    json::JArray* blocksarr = root->arr("blocks");
    if (blocksarr) {
        cout << "    blocks: " << blocksarr->size() << endl;
        for (uint i = 0; i < blocksarr->size(); i++) {
            string name = blocksarr->str(i); 
            cout << "    loading block " << id << ":" << name << endl;
            path blockfile = folder/path("blocks/"+name+".json");
            builder->add(loadBlock(id+":"+name, blockfile));
        }
    }
}
