#include "WorldFiles.h"

#include "rle.h"
#include "binary_io.h"
#include "../window/Camera.h"
#include "../content/Content.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../typedefs.h"
#include "../maths/voxmaths.h"
#include "../world/World.h"
#include "../lighting/Lightmap.h"

#include "../util/data_io.h"
#include "../coders/json.h"
#include "../constants.h"
#include "../items/ItemDef.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cstring>

namespace fs = std::filesystem;

regfile::regfile(fs::path filename) : file(filename) {
    if (file.length() < REGION_HEADER_SIZE)
        throw std::runtime_error("incomplete region file header");
    char header[REGION_HEADER_SIZE];
    file.read(header, REGION_HEADER_SIZE);
    
    // avoid of use strcmp_s
    if (std::string(header, strlen(REGION_FORMAT_MAGIC)) != REGION_FORMAT_MAGIC) {
        throw std::runtime_error("invalid region file magic number");
    }
    version = header[8];
    if (uint(version) > REGION_FORMAT_VERSION) {
        throw illegal_region_format(
            "region format "+std::to_string(version)+" is not supported");
    }
}

void WorldRegion::setUnsaved(bool unsaved) {
	this->unsaved = unsaved;
}
bool WorldRegion::isUnsaved() const {
	return unsaved;
}

std::array<std::optional<std::vector<ubyte>>, REGION_CHUNKS_COUNT>& WorldRegion::getChunks() /*const*/ {
	return chunks;
}

void WorldRegion::put(uint x, uint z, std::vector<ubyte>&& data) {
	size_t chunk_index = z * REGION_SIZE + x;
	chunks[chunk_index] = std::forward<std::vector<ubyte>>(data);
}

std::optional<std::vector<ubyte>>& WorldRegion::getChunkData(uint x, uint z) {
	return chunks[z * REGION_SIZE + x];
}

WorldFiles::WorldFiles(fs::path directory, const DebugSettings& settings) 
	: directory(directory), 
	  generatorTestMode(settings.generatorTestMode),
	  doWriteLights(settings.doWriteLights) {
}

WorldFiles::~WorldFiles() = default;

WorldRegion* WorldFiles::getRegion(RegionsMap& regions, int x, int z) {
	auto found = regions.find(glm::ivec2(x, z));
	if (found == regions.end())
		return nullptr;
	return found->second.get();
}

WorldRegion* WorldFiles::getOrCreateRegion(RegionsMap& regions, int x, int z) {
	WorldRegion* region = getRegion(regions, x, z);
	if (region == nullptr) {
		region = new WorldRegion();
		regions[glm::ivec2(x, z)].reset(region);
	}
	return region;
}

std::vector<ubyte> WorldFiles::compress(const ubyte* src, size_t srclen) {
    std::vector<ubyte> data(CHUNK_DATA_LEN * 2);
    auto len = extrle::encode(src, srclen, data.data());
    data.resize(len);
    return data;
}

std::vector<ubyte> WorldFiles::decompress(const std::vector<ubyte>& src, size_t dstlen) {
    std::vector<ubyte> decompressed(dstlen);
    auto decompressed_size = extrle::decode(src.data(), src.size(), decompressed.data());
    if (decompressed.size() > decompressed_size) {
        decompressed.resize(decompressed_size);
    }
    return decompressed;
}

int WorldFiles::getVoxelRegionVersion(int x, int z) {
    regfile* rf = getRegFile(glm::ivec3(x, z, REGION_LAYER_VOXELS), getRegionsFolder());
    if (rf == nullptr) {
        return 0;
    }
    return rf->version;
}

int WorldFiles::getVoxelRegionsVersion() {
    fs::path regionsFolder = getRegionsFolder();
    if (!fs::is_directory(regionsFolder)) {
        return REGION_FORMAT_VERSION;
    }
    for (auto file : fs::directory_iterator(regionsFolder)) {
        int x;
        int z;
        if (!parseRegionFilename(file.path().stem().string(), x, z)) {
            continue;
        }
        regfile* rf = getRegFile(glm::ivec3(x, z, REGION_LAYER_VOXELS), regionsFolder);
        return rf->version;
    }
    return REGION_FORMAT_VERSION;
}

/* 
 * Compress and store chunk voxels data in region 
 * @param x chunk.x
 * @param z chunk.z
 */
void WorldFiles::put(int x, int z, const ubyte* voxelData) {
    int regionX = floordiv(x, REGION_SIZE);
	int regionZ = floordiv(z, REGION_SIZE);
	int localX = x - (regionX * REGION_SIZE);
	int localZ = z - (regionZ * REGION_SIZE);

	/* Writing Voxels */ {
		WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
		region->setUnsaved(true);
		auto data = compress(voxelData, CHUNK_DATA_LEN);
		region->put(localX, localZ, std::move(data));
	}
}

/*
 * Store chunk (voxels and lights) in region (existing or new)
 */
void WorldFiles::put(Chunk* chunk){
	assert(chunk != nullptr);

	int regionX = floordiv(chunk->x, REGION_SIZE);
	int regionZ = floordiv(chunk->z, REGION_SIZE);
	int localX = chunk->x - (regionX * REGION_SIZE);
	int localZ = chunk->z - (regionZ * REGION_SIZE);

	/* Writing Voxels */ {
        std::unique_ptr<ubyte[]> chunk_data (chunk->encode());
		auto data = compress(chunk_data.get(), CHUNK_DATA_LEN);

		WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
		region->setUnsaved(true);
		region->put(localX, localZ, std::move(data));
	}
	if (doWriteLights && chunk->isLighted()) {
        auto light_data = chunk->lightmap->encode();
		auto data = compress(light_data.data(), light_data.size());

		WorldRegion* region = getOrCreateRegion(lights, regionX, regionZ);
		region->setUnsaved(true);
		region->put(localX, localZ, std::move(data));
	}
}

fs::path WorldFiles::getRegionsFolder() const {
	return directory/fs::path("regions");
}

fs::path WorldFiles::getLightsFolder() const {
	return directory/fs::path("lights");
}

fs::path WorldFiles::getRegionFilename(int x, int z) const {
	return fs::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

/* 
 * Extract X and Z from 'X_Z.bin' region file name.
 * @param name source region file name
 * @param x parsed X destination
 * @param z parsed Z destination
 * @return false if std::invalid_argument or std::out_of_range occurred
 */
bool WorldFiles::parseRegionFilename(const std::string& name, int& x, int& z) {
    size_t sep = name.find('_');
    if (sep == std::string::npos || sep == 0 || sep == name.length()-1)
        return false;
    try {
        x = std::stoi(name.substr(0, sep));
        z = std::stoi(name.substr(sep+1));
    } catch (std::invalid_argument& err) {
        return false;
    } catch (std::out_of_range& err) {
        return false;
    }
    return true;
}

fs::path WorldFiles::getPlayerFile() const {
	return directory/fs::path("player.json");
}

fs::path WorldFiles::getWorldFile() const {
	return directory/fs::path("world.json");
}

fs::path WorldFiles::getIndicesFile() const {
	return directory/fs::path("indices.json");
}

fs::path WorldFiles::getPacksFile() const {
	return directory/fs::path("packs.list");
}

std::optional<std::vector<ubyte>> WorldFiles::getChunk(int x, int z) {
    return getData(regions, getRegionsFolder(), x, z, REGION_LAYER_VOXELS);
}

/* Get cached lights for chunk at x,z
 * @return lights data or nullptr */
std::optional<std::vector<light_t>> WorldFiles::getLights(int x, int z) {
    auto data = getData(lights, getLightsFolder(), x, z, REGION_LAYER_LIGHTS);
    if (!data.has_value()) {
        return std::nullopt;
    }
    return Lightmap::decode(data.value());
}

std::optional<std::vector<ubyte>> WorldFiles::getData(RegionsMap& regions,
                                                      const fs::path& folder,
                                                      int x, int z, int layer) {
	int regionX = floordiv(x, REGION_SIZE);
	int regionZ = floordiv(z, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localZ = z - (regionZ * REGION_SIZE);

	WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
	auto& chunk = region->getChunkData(localX, localZ);
	if (!chunk.has_value()) {
		chunk = readChunkData(x, z, folder, layer);
	}
	if (chunk.has_value()) {
		return decompress(chunk.value(), CHUNK_DATA_LEN);
	}
	return std::nullopt;
}


regfile* WorldFiles::getRegFile(glm::ivec3 coord, const fs::path& folder) {
    const auto found = openRegFiles.find(coord);
    if (found != openRegFiles.end()) {
        return found->second.get();
    }
    if (openRegFiles.size() == MAX_OPEN_REGION_FILES) {
        // [todo] replace with closing the most unused region
        auto iter = std::next(openRegFiles.begin(), rand() % openRegFiles.size());
        openRegFiles.erase(iter);
    }
	fs::path filename = folder / getRegionFilename(coord[0], coord[1]);
    if (!fs::is_regular_file(filename)) {
        return nullptr;
    }
    openRegFiles[coord] = std::make_unique<regfile>(filename);
    return openRegFiles[coord].get();
}

std::optional<std::vector<ubyte>> WorldFiles::readChunkData(int x, int z,
                                                            fs::path folder,
                                                            int layer) {
	if (generatorTestMode)
		return std::nullopt;

	int regionX = floordiv(x, REGION_SIZE);
	int regionZ = floordiv(z, REGION_SIZE);
	int localX = x - (regionX * REGION_SIZE);
	int localZ = z - (regionZ * REGION_SIZE);
	int chunkIndex = localZ * REGION_SIZE + localX;
 
    glm::ivec3 coord(regionX, regionZ, layer);
    regfile* rfile = WorldFiles::getRegFile(coord, folder);
    if (rfile == nullptr) {
        return std::nullopt;
    }
    files::rafile& file = rfile->file;

	size_t file_size = file.length();
	size_t table_offset = file_size - REGION_CHUNKS_COUNT * 4;

	uint32_t offset;
	file.seekg(table_offset + chunkIndex * 4);
	file.read((char*)(&offset), 4);
	offset = dataio::read_int32_big((const ubyte*)(&offset), 0);
	if (offset == 0){
		return std::nullopt;
	}
	file.seekg(offset);
	file.read((char*)(&offset), 4);
	auto length = dataio::read_int32_big((const ubyte*)(&offset), 0);
	std::vector<ubyte> data(length);
	file.read((char*)data.data(), data.size());
	if (data.data() == nullptr) { // FIXME: this condition is always TRUE! There is should be another way for checking read status.
		std::cerr << "ERROR: failed to read data of chunk x("<< x <<"), z("<< z <<")" << std::endl;
	}
	return data;
}

/* Read missing chunks data (null pointers) from region file 
 * @param layer used as third part of openRegFiles map key 
 * (see REGION_LAYER_* constants)
 */
void WorldFiles::fetchChunks(WorldRegion* region, int x, int z, fs::path folder, int layer) {
	auto& chunks = region->getChunks();

    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        int chunk_x = (i % REGION_SIZE) + x * REGION_SIZE;
        int chunk_z = (i / REGION_SIZE) + z * REGION_SIZE;
        if (!chunks[i].has_value()) {
            chunks[i] = readChunkData(chunk_x, chunk_z, folder, layer);
        }
    }
}

/* Write or rewrite region file
 * @param x region X
 * @param z region Z
 * @param layer used as third part of openRegFiles map key 
 * (see REGION_LAYER_* constants)
 */
void WorldFiles::writeRegion(int x, int z, WorldRegion* entry, fs::path folder, int layer){
    fs::path filename = folder/getRegionFilename(x, z);

    glm::ivec3 regcoord(x, z, layer);
    if (getRegFile(regcoord, folder)) {
        fetchChunks(entry, x, z, folder, layer);
        openRegFiles.erase(regcoord);
    }
    
	char header[REGION_HEADER_SIZE] = REGION_FORMAT_MAGIC;
	header[8] = REGION_FORMAT_VERSION;
	header[9] = 0; // flags
	std::ofstream file(filename, std::ios::out | std::ios::binary);
	file.write(header, REGION_HEADER_SIZE);

	size_t offset = REGION_HEADER_SIZE;
	char intbuf[4]{};
	uint offsets[REGION_CHUNKS_COUNT]{};
	
    const auto& region = entry->getChunks();

	for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
		const auto& chunk = region[i];
		if (!chunk.has_value()) {
			offsets[i] = 0;
		} else {
			offsets[i] = offset;

			size_t compressedSize = chunk.value().size();
			dataio::write_int32_big(compressedSize, (ubyte*)intbuf, 0);
			offset += 4 + compressedSize;

			file.write(intbuf, 4);
			file.write((const char*)chunk.value().data(), compressedSize);
		}
	}
	for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
		dataio::write_int32_big(offsets[i], (ubyte*)intbuf, 0);
		file.write(intbuf, 4);
	}
}

void WorldFiles::writeRegions(RegionsMap& regions, const fs::path& folder, int layer) {
	for (auto& it : regions){
		WorldRegion* region = it.second.get();
		if (!region->isUnsaved())
			continue;
		glm::ivec2 key = it.first;
		writeRegion(key[0], key[1], region, folder, layer);
	}
}

void WorldFiles::write(const World* world, const Content* content) {
	fs::path regionsFolder = getRegionsFolder();
	fs::path lightsFolder = getLightsFolder();

	fs::create_directories(regionsFolder);
	fs::create_directories(lightsFolder);

    if (world) {
	    writeWorldInfo(world);
		writePacks(world);
	}
	if (generatorTestMode)
		return;
		
	writeIndices(content->indices);
	writeRegions(regions, regionsFolder, REGION_LAYER_VOXELS);
	writeRegions(lights, lightsFolder, REGION_LAYER_LIGHTS);
}

void WorldFiles::writePacks(const World* world) {
	const auto& packs = world->getPacks();
	std::stringstream ss;
	ss << "# autogenerated; do not modify\n";
	for (const auto& pack : packs) {
		ss << pack.id << "\n";
	}
	files::write_string(getPacksFile(), ss.str());
}

void WorldFiles::writeIndices(const ContentIndices* indices) {
	json::JObject root;
    uint count;
	json::JArray& blocks = root.putArray("blocks");
	count = indices->countBlockDefs();
	for (uint i = 0; i < count; i++) {
		const Block* def = indices->getBlockDef(i);
		blocks.put(def->name);
	}

    json::JArray& items = root.putArray("items");
	count = indices->countItemDefs();
	for (uint i = 0; i < count; i++) {
		const ItemDef* def = indices->getItemDef(i);
		items.put(def->name);
	}

	files::write_string(getIndicesFile(), json::stringify(&root, true, "  "));
}

void WorldFiles::writeWorldInfo(const World* world) {
	json::JObject root;

	json::JObject& versionobj = root.putObj("version");
	versionobj.put("major", ENGINE_VERSION_MAJOR);
	versionobj.put("minor", ENGINE_VERSION_MINOR);

	root.put("name", world->name);
	root.put("seed", world->seed);
	
	json::JObject& timeobj = root.putObj("time");
	timeobj.put("day-time", world->daytime);
	timeobj.put("day-time-speed", world->daytimeSpeed);

	files::write_string(getWorldFile(), json::stringify(&root, true, "  "));
}

bool WorldFiles::readWorldInfo(World* world) {
	fs::path file = getWorldFile();
	if (!fs::is_regular_file(file)) {
		std::cerr << "warning: world.json does not exists" << std::endl;
		return false;
	}

	std::unique_ptr<json::JObject> root(files::read_json(file));
	root->str("name", world->name);
	root->num("seed", world->seed);

	json::JObject* verobj = root->obj("version");
	if (verobj) {
		int major=0, minor=-1;
		verobj->num("major", major);
		verobj->num("minor", minor);
		std::cout << "world version: " << major << "." << minor << std::endl;
	}

	json::JObject* timeobj = root->obj("time");
	if (timeobj) {
		timeobj->num("day-time", world->daytime);
		timeobj->num("day-time-speed", world->daytimeSpeed);
	}

	return true;
}

void WorldFiles::writePlayer(Player* player){
	glm::vec3 position = player->hitbox->position;
	json::JObject root;
	json::JArray& posarr = root.putArray("position");
	posarr.put(position.x);
	posarr.put(position.y);
	posarr.put(position.z);

	json::JArray& rotarr = root.putArray("rotation");
	rotarr.put(player->cam.x);
	rotarr.put(player->cam.y);
	
	root.put("flight", player->flight);
	root.put("noclip", player->noclip);

	files::write_string(getPlayerFile(), json::stringify(&root, true, "  "));
}

bool WorldFiles::readPlayer(Player* player) {
	fs::path file = getPlayerFile();
	if (!fs::is_regular_file(file)) {
		std::cerr << "warning: player.json does not exists" << std::endl;
		return false;
	}

	std::unique_ptr<json::JObject> root(files::read_json(file));
	json::JArray* posarr = root->arr("position");
	glm::vec3& position = player->hitbox->position;
	position.x = posarr->num(0);
	position.y = posarr->num(1);
	position.z = posarr->num(2);
	player->camera->position = position;

	json::JArray* rotarr = root->arr("rotation");
	player->cam.x = rotarr->num(0);
	player->cam.y = rotarr->num(1);

	root->flag("flight", player->flight);
	root->flag("noclip", player->noclip);
	return true;
}
