#include "WorldGenerator.h"

#include "../content/Content.h"

WorldGenerator::WorldGenerator(const Content* content)
               : idStone(content->requireBlock("base:stone").rt.id),
                 idDirt(content->requireBlock("base:dirt").rt.id),
                 idGrassBlock(content->requireBlock("base:grass_block").rt.id),
                 idSand(content->requireBlock("base:sand").rt.id),
                 idWater(content->requireBlock("base:water").rt.id),
                 idWood(content->requireBlock("base:wood").rt.id),
                 idLeaves(content->requireBlock("base:leaves").rt.id),
                 idGrass(content->requireBlock("base:grass").rt.id),
                 idFlower(content->requireBlock("base:flower").rt.id),
                 idBazalt(content->requireBlock("base:bazalt").rt.id) {}