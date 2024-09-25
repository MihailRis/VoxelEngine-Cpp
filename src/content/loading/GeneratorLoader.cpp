#include "../ContentLoader.hpp"

#include "../ContentPack.hpp"

#include "files/files.hpp"
#include "logic/scripting/scripting.hpp"
#include "world/generator/GeneratorDef.hpp"
#include "world/generator/VoxelFragment.hpp"
#include "debug/Logger.hpp"

static debug::Logger logger("generator-loader");

static VoxelStructureMeta load_structure_meta(
    const std::string& name, const dv::value& config
) {
    VoxelStructureMeta meta;
    meta.name = name;

    return meta;
}

static std::vector<std::unique_ptr<GeneratingVoxelStructure>> load_structures(
    const fs::path& structuresFile
) {
    auto structuresDir = structuresFile.parent_path() / fs::path("fragments");
    auto map = files::read_json(structuresFile);

    std::vector<std::unique_ptr<GeneratingVoxelStructure>> structures;
    for (auto& [name, config] : map.asObject()) {
        auto structFile = structuresDir / fs::u8path(name + ".vox");
        logger.debug() << "loading voxel fragment " << structFile.u8string();
        if (!fs::exists(structFile)) {
            throw std::runtime_error("structure file does not exist (" +
                structFile.u8string());
        }
        auto fragment = std::make_unique<VoxelFragment>();
        fragment->deserialize(files::read_binary_json(structFile));

        structures.push_back(std::make_unique<GeneratingVoxelStructure>(
            load_structure_meta(name, config),
            std::move(fragment)
        ));
    }
    return structures;
}

static void load_structures(GeneratorDef& def, const fs::path& structuresFile) {
    auto rawStructures = load_structures(structuresFile);
    def.structures.resize(rawStructures.size());

    for (int i = 0; i < rawStructures.size(); i++) {
        def.structures[i] = std::move(rawStructures[i]);
    }
    // build indices map
    for (size_t i = 0; i < def.structures.size(); i++) {
        auto& structure = def.structures[i];
        def.structuresIndices[structure->meta.name] = i;
    }
}

static inline const auto STRUCTURES_FILE = fs::u8path("structures.json");
static inline const auto GENERATORS_DIR = fs::u8path("generators");

void ContentLoader::loadGenerator(
    GeneratorDef& def, const std::string& full, const std::string& name
) {
    auto packDir = pack->folder;
    auto generatorsDir = packDir / GENERATORS_DIR;
    auto folder = generatorsDir / fs::u8path(name);
    auto generatorFile = generatorsDir / fs::u8path(name + ".lua");
    if (!fs::exists(generatorFile)) {
        return;
    }
    auto structuresFile = folder / STRUCTURES_FILE;
    if (fs::exists(structuresFile)) {
        load_structures(def, structuresFile);
    }
    def.script = scripting::load_generator(generatorFile);
}
