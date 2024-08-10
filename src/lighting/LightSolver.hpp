#pragma once

#include <queue>

class Chunks;
class ContentIndices;

struct lightentry {
    int x;
    int y;
    int z;
    unsigned char light;
};

class LightSolver {
    std::queue<lightentry> addqueue;
    std::queue<lightentry> remqueue;
    const ContentIndices* const contentIds;
    Chunks* chunks;
    int channel;
public:
    LightSolver(const ContentIndices* contentIds, Chunks* chunks, int channel);

    void add(int x, int y, int z);
    void add(int x, int y, int z, int emission);
    void remove(int x, int y, int z);
    void solve();
};
