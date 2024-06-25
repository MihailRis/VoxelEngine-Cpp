#include "WorldGenerators.hpp"
#include "../voxels/WorldGenerator.hpp"
#include "../voxels/FlatWorldGenerator.hpp"
#include "../voxels/SkygridWorldGenerator.hpp"
#include "../content/Content.hpp"
#include <iostream>

std::vector<std::string> WorldGenerators::getGeneratorsIDs() {
    std::vector<std::string> ids;
    for (auto& entry : generators) {
        ids.push_back(entry.first);
    }
    return ids;
}

std::string WorldGenerators::getDefaultGeneratorID() {
    return "core:default";
}

std::unique_ptr<WorldGenerator> WorldGenerators::createGenerator(const std::string& id, const Content* content) {
    auto found = generators.find(id);
    if (found == generators.end()) {
        throw std::runtime_error("unknown generator id: "+id);
    }
    return std::unique_ptr<WorldGenerator>(found->second(content));
}
