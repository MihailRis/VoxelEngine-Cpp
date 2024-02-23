#include "WorldGenerators.h"
#include "../voxels/WorldGenerator.h"
#include "../voxels/FlatWorldGenerator.h"
#include "../content/Content.h"
#include <vector>
#include <string>
#include <iostream>
#include <function>

std::map<std::string, std::function> generators;

template <typename T> 
void WorldGenerators::addGenerator(std::string id) {
    using create_func = std::function<T*>(const Content*);
    generators[id] = create_func
}

std::vector<std::string> WorldGenerators::getGeneratorsIDs() {
    std::vector<std::string> ids;

    for(std::map<std::string, std::function>::iterator it = generators.begin(); it != generators.end(); ++it) {
        ids.push_back(it->first);
    }

    return ids;
}

std::string WorldGenerators::getDefaultWorldGeneratorID() {
    return "core:default";
}

WorldGenerator* WorldGenerators::createWorldGenerator(std::string id, const Content* content) {
    for(std::map<std::string, std::function>::iterator it = generators.begin(); it != generators.end(); ++it) {
        if(id == it->first) {
            return (WorldGenerator*) it->second(content);
        }
    }

    std::cerr << "unknown generator id: " << id << std::endl;
    return nullptr;
}