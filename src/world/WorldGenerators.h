#ifndef WORLD_WORLDTYPES_H_
#define WORLD_WORLDTYPES_H_

#include "../voxels/WorldGenerator.h"
#include "../content/Content.h"
#include <vector>
#include <string>

class WorldGenerators {

public:
    static void addGenerator(std::string id);

    static std::vector<std::string> getGeneratorsIDs();

    static std::string getDefaultGeneratorID();

    static WorldGenerator* createGenerator(std::string id, const Content* content);
};

#endif /* WORLD_WORLDTYPES_H_ */