#include "WorldTypes.h"
#include "../voxels/WorldGenerator.h"
#include "../voxels/FlatWorldGenerator.h"
#include "../content/Content.h"
#include <vector>
#include <string>
#include <iostream>

std::vector<std::string> worldTypes;

void fillTypes() {
    worldTypes.push_back("Default");
    worldTypes.push_back("Flat");
}

std::vector<std::string> WorldTypes::getWorldTypes() {
    if(worldTypes.size() == 0) {
        fillTypes();
    }

    return worldTypes;
}

std::string WorldTypes::getDefaultWorldType() {
    return getWorldTypes()[0];
}

WorldGenerator* WorldTypes::createWorldGenerator(std::string worldType, const Content* content) {

    if(worldType == "Default") {
        return new WorldGenerator(content);
    } else if(worldType == "Flat") {
        return (WorldGenerator*) new FlatWorldGenerator(content);
    }
    std::cerr << "unknown world type: " << worldType << std::endl;
    return nullptr;
}