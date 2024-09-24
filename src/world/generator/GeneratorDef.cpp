#include "GeneratorDef.hpp"

#include "VoxelFragment.hpp"

GeneratingVoxelStructure::GeneratingVoxelStructure(
    VoxelStructureMeta meta,
    std::unique_ptr<VoxelFragment> structure
) : structure(std::move(structure)), meta(std::move(meta)) {}


GeneratorDef::GeneratorDef(std::string name) : name(std::move(name)) {}
