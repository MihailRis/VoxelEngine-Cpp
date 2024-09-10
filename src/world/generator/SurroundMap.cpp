#include "SurroundMap.hpp"

#include <queue>
#include <cstring>
#include <stdexcept>

SurroundMap::SurroundMap(int loadDistance, ubyte maxLevel) 
    : areaMap((loadDistance + maxLevel) * 2 + 1, 
              (loadDistance + maxLevel) * 2 + 1),
      levelCallbacks(maxLevel),
      maxLevel(maxLevel) 
{}

void SurroundMap::setLevelCallback(int level, LevelCallback callback) {
    levelCallbacks.at(level) = callback;
}

void SurroundMap::setOutCallback(util::AreaMap2D<ubyte>::OutCallback callback) {
    areaMap.setOutCallback(callback);
}

void SurroundMap::completeAt(int x, int y) {
    // TODO
}

void SurroundMap::setCenter(int x, int y) {
    areaMap.setCenter(x, y);
}
