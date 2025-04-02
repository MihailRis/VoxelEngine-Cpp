#include "GeneratorDef.hpp"

#include "VoxelFragment.hpp"
#include "content/Content.hpp"
#include "util/stringutil.hpp"
#include "voxels/Block.hpp"

VoxelStructure::VoxelStructure(
    VoxelStructureMeta meta,
    std::unique_ptr<VoxelFragment> structure
) : meta(std::move(meta)), fragments({std::move(structure)}) {}

GeneratorDef::GeneratorDef(std::string name)
    : name(std::move(name)), caption(util::id_to_caption(name)) {
}

void GeneratorDef::prepare(const Content* content) {
    for (auto& biome : biomes) {
        for (auto& layer : biome.groundLayers.layers) {
            layer.rt.id = content->blocks.require(layer.block).rt.id;
        }
        for (auto& layer : biome.seaLayers.layers) {
            layer.rt.id = content->blocks.require(layer.block).rt.id;
        }
        for (auto& plant : biome.plants.entries) {
            plant.rt.id = content->blocks.require(plant.name).rt.id;
        }
        for (auto& structure : biome.structures.entries) {
            const auto& found = structuresIndices.find(structure.name);
            if (found == structuresIndices.end()) {
                throw std::runtime_error(
                    "no structure "+util::quote(structure.name)+" found");
            }
            structure.rt.id = found->second;
        }
    }
}
