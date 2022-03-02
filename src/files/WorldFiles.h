#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <unordered_map>

class Player;

#define REGION_SIZE_BIT 5
#define REGION_SIZE (1 << (REGION_SIZE_BIT))
#define REGION_VOL ((REGION_SIZE) * (REGION_SIZE))

/* Требование:
 * - высота мира = 1 чанк (любых размеров)
 * Пример:
 * - CHUNK_W = 16, CHUNK_H = 128, CHUNK_D = 16
 * */
class WorldFiles {
public:
	static unsigned long totalCompressed;
	std::unordered_map<long, char**> regions;
	std::string directory;
	char* mainBufferIn;
	char* mainBufferOut;

	WorldFiles(const char* directory, size_t mainBufferCapacity);
	~WorldFiles();

	void put(const char* chunkData, int x, int y);

	bool readPlayer(Player* player);
	bool readChunk(int x, int y, char* out);
	bool getChunk(int x, int y, char* out);
	void readRegion(char* fileContent);
	unsigned int writeRegion(char* out, int x, int y, char** region);
	void writePlayer(Player* player);
	void write();

	std::string getRegionFile(int x, int y);
	std::string getPlayerFile();
};

extern void longToCoords(int& x, int& y, long key);

#endif /* FILES_WORLDFILES_H_ */
