#ifndef WORLD_WORLDTYPES_H_
#define WORLD_WORLDTYPES_H_

#include "../voxels/WorldGenerator.h"
#include "../content/Content.h"
#include <vector>
#include <string>

class WorldTypes {
public:
    static std::vector<std::string> getWorldTypes();

    static std::string getDefaultWorldType();

    static WorldGenerator* createWorldGenerator(std::string worldType, const Content* content);
};

#endif /* WORLD_WORLDTYPES_H_ */