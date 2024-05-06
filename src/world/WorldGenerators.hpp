#ifndef WORLD_WORLDGENERATORS_HPP_
#define WORLD_WORLDGENERATORS_HPP_

#include "../voxels/WorldGenerator.hpp"
#include "../content/Content.hpp"
#include <map>
#include <vector>
#include <string>

typedef WorldGenerator* (*gen_constructor) (const Content*);


class WorldGenerators {
    static inline std::map<std::string, gen_constructor> generators;

public:
    template <typename T>
    static void addGenerator(std::string id);

    static std::vector<std::string> getGeneratorsIDs();

    static std::string getDefaultGeneratorID();

    static std::unique_ptr<WorldGenerator> createGenerator(
        std::string id, const Content* content
    );
};

template <typename T>
void WorldGenerators::addGenerator(std::string id) {
    generators[id] = 
    [] (const Content* content)
    {
        return (WorldGenerator*) new T(content);
    };
}

#endif /* WORLD_WORLDGENERATORS_HPP_ */
