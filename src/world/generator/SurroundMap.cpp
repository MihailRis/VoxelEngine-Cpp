#include "SurroundMap.hpp"

#include <queue>
#include <cstring>
#include <stdexcept>

void SurroundMap::resetMarks() {
    for (auto& [_, entry] : entries) {
        entry.marked = false;
    }
}

bool SurroundMap::createEntry(const glm::ivec2& origin) {
    auto& entry = entries[origin];
    if (entry.confirmed) {
        return false;
    }
    entry.confirmed = true;

    // calculate initial number of surrounding entries (any level)
    int bitOffset = 0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            if (x == y && x == 0) {
                continue;
            }
            const auto& found = entries.find(origin + glm::ivec2(x, y));
            if (found != entries.end()) {
                entry.surrounding |= (1 << bitOffset);
                if (found->second.level == 0) {
                    found->second.surrounding |= (1 << (7 - bitOffset));
                }
            }
            bitOffset++;
        }
    }
    return true;
}

std::vector<glm::ivec2> SurroundMap::upgrade() {
    std::vector<glm::ivec2> expansion;
    std::queue<glm::ivec2> expanding;
    for (const auto& [pos, entry] : entries) {
        if (entry.confirmed && entry.surrounding != 0xFF) {
            expanding.push(pos);
        }
    }
    while (!expanding.empty()) {
        assert(expanding.size() < 64);

        glm::ivec2 pos = expanding.front();
        expanding.pop();

        const auto& found = entries.find(pos);
        assert(found != entries.end() && "concurrent modification");

        auto& entry = found->second;
        int uplevelSurrounding = 0;
        int bitOffset = 0;
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                if (x == y && x == 0) {
                    continue;
                }
                glm::ivec2 npos = {pos.x+x, pos.y+y};
                const auto& nfound = entries.find(npos);
                
                if (entry.surrounding & (1 << bitOffset)) {
                    auto& nentry = nfound->second;
                    if (nentry.level > entry.level) {
                        uplevelSurrounding |= (1 << bitOffset);
                    }
                    bitOffset++;
                    continue;
                }
                if (entry.level == 0) {
                    // neighbour entry does not exist
                    createEntry(npos);
                    expansion.push_back(npos);
                } else{
                    assert(nfound != entries.end() && "invalid map state");
                    if (nfound->second.level == entry.level + 1) {
                        nfound->second.surrounding |= (1 << (7 - bitOffset));
                        expanding.push(npos);
                    }
                }
                bitOffset++;
            }
        }
        // level up
        entry.surrounding = uplevelSurrounding;
        entry.level++;
    }
    return expansion;
}

void SurroundMap::getLevels(unsigned char* out, int width, int height, int ox, int oy) const {
    std::memset(out, 0, width * height);
    for (const auto& [pos, entry] : entries) {
        int x = pos.x - ox;
        int y = pos.y - oy;
        if (x < 0 || x >= width || y < 0 || y >= height) {
            continue;
        }

        int surroundNum = 0;
        for (int i = 0; i < 8; i++) {
            if (entry.surrounding & (1 << i)) {
                surroundNum++;
            }
        }
        if (surroundNum) {
            out[y * width + x] = surroundNum + 1;
        }
        out[y * width + x] = entry.level + 1;
    }
}
