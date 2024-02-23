#include "WorldGenerators.h"
#include "../voxels/WorldGenerator.h"
#include "../voxels/FlatWorldGenerator.h"
#include "../content/Content.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>

std::vector<std::string> WorldGenerators::getGeneratorsIDs() {
    std::vector<std::string> ids;

    for(std::map<std::string, gen_constructor>::iterator it = generators.begin(); it != generators.end(); ++it) {
        ids.push_back(it->first);
    }

    return ids;
}

std::string WorldGenerators::getDefaultGeneratorID() {
    return "core:default";
}

WorldGenerator* WorldGenerators::createGenerator(std::string id, const Content* content) {
    for(std::map<std::string, gen_constructor>::iterator it = generators.begin(); it != generators.end(); ++it) {
        if(id == it->first) {
            return (WorldGenerator*) it->second(content);
        }
    }

    std::cerr << "unknown generator id: " << id << std::endl;
    return nullptr;
}