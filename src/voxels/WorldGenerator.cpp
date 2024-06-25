#include "WorldGenerator.hpp"
#include "voxel.hpp"
#include "Chunk.hpp"
#include "Block.hpp"

#include "../content/Content.hpp"

WorldGenerator::WorldGenerator(const Content* content)
  : idStone(content->blocks.require("base:stone").rt.id),
    idDirt(content->blocks.require("base:dirt").rt.id),
    idGrassBlock(content->blocks.require("base:grass_block").rt.id),
    idSand(content->blocks.require("base:sand").rt.id),
    idWater(content->blocks.require("base:water").rt.id),
    idWood(content->blocks.require("base:wood").rt.id),
    idLeaves(content->blocks.require("base:leaves").rt.id),
    idGrass(content->blocks.require("base:grass").rt.id),
    idFlower(content->blocks.require("base:flower").rt.id),
    idBazalt(content->blocks.require("base:bazalt").rt.id) {}
