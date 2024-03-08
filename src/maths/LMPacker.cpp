#include "LMPacker.h"

#include <algorithm>

inline int getPackerScore(rectangle& rect) {
    if (rect.width * rect.height > 100)
        return rect.height * rect.height * 1000;
    return (rect.width * rect.height * rect.height);
}

LMPacker::LMPacker(const uint32_t sizes[], size_t length) {
    for (unsigned int i = 0; i < length/2; i++) {
        rectangle rect(i, 0, 0, (int)sizes[i * 2], (int)sizes[i * 2 + 1]);
        rects.push_back(rect);
    }
    sort(rects.begin(), rects.end(), [](rectangle a, rectangle b) {
        return -getPackerScore(a) < -getPackerScore(b);
    });
}

LMPacker::~LMPacker() {
    cleanup();
}

void LMPacker::cleanup() {
    if (matrix) {
        for (unsigned int y = 0; y < (height >> mbit); y++) {
            delete[] matrix[y];
        }
        delete[] matrix;
    }
    placed.clear();
}

bool LMPacker::build(uint32_t width, uint32_t height, 
                     uint16_t extension, uint32_t mbit, uint32_t vstep) {
    cleanup();
    this->mbit = mbit;
    this->width = width;
    this->height = height;
    int mpix = 1 << mbit;

    const unsigned int mwidth = width >> mbit;
    const unsigned int mheight = height >> mbit;

    matrix = new rectangle**[mheight];
    for (unsigned int y = 0; y < mheight; y++) {
        matrix[y] = new rectangle*[mwidth];
        for (unsigned int x = 0; x < mwidth; x++) {
            matrix[y][x] = nullptr;
        }
    }

    for (unsigned int i = 0; i < rects.size(); i++) {
        rectangle& rect = rects[i];
        rect = rectangle(rect.idx, 0, 0, rect.width, rect.height);
        rect.width += extension * 2;
        rect.height += extension * 2;
        if (mpix > 1) {
            if (rect.width % mpix > 0) {
                 rect.extX = mpix - (rect.width % mpix);
            }
            if (rect.height % mpix > 0) {
                rect.extY = mpix - (rect.height % mpix);
            }
        }
        rect.width += rect.extX;
        rect.height += rect.extY;
    }
    bool built = true;
    for (unsigned int i = 0; i < rects.size(); i++) {
        rectangle* rect = &rects[i];
        if (!place(rect, vstep)) {
            built = false;
            break;
        }
    }
    for (unsigned int i = 0; i < rects.size(); i++) {
        rectangle& rect = rects[i];
        rect.x += extension;
        rect.y += extension;
        rect.width -= extension * 2 + rect.extX;
        rect.height -= extension * 2 + rect.extY;
    }
    return built;
}

inline rectangle* findCollision(rectangle*** matrix, int x, int y, int w, int h) {
    for (int row = y; row < y+h; row++) {
        for (int col = x; col < x+w; col++) {
            rectangle* rect = matrix[row][col];
            if (rect) {
                return rect;
            }
        }
    }
    return nullptr;
}

inline void fill(rectangle*** matrix, rectangle* rect, int x, int y, int w, int h) {
    for (int row = y; row < y+h; row++) {
        for (int col = x; col < x+w; col++) {
            matrix[row][col] = rect;
        }
    }
}

bool LMPacker::place(rectangle* rectptr, uint32_t vstep) {
    rectangle& rect = *rectptr;
    const unsigned int rw = rect.width >> mbit;
    const unsigned int rh = rect.height >> mbit;
    if (vstep > 1) {
        vstep = (vstep > rh ? vstep : rh);
    }
    const unsigned int mwidth = width >> mbit;
    const unsigned int mheight = height >> mbit;
    for (unsigned int y = 0; y + rh < mheight; y += vstep) {
        rectangle** line = matrix[y];
        bool skiplines = true;
        rectangle** lower = matrix[y + rh - 1];
        for (unsigned int x = 0; x + rw < mwidth; x++) {
            rectangle* prect = line[x];
            if (prect) {
                x = (prect->x >> mbit) + (prect->width >> mbit) - 1;
            } else {
                if (skiplines) {
                    unsigned int lfree = 0;
                    while (lfree + x < mwidth && !lower[x + lfree] && lfree < rw) {
                        lfree++;
                    }
                    if (lfree >= rw)
                        skiplines = false;
                }
                prect = findCollision(matrix, x, y, rw, rh);
                if (prect) {
                    x = (prect->x >> mbit) + (prect->width >> mbit) - 1;
                    continue;
                }
                fill(matrix, rectptr, x, y, rw, rh);
                rectptr->x = x << mbit;
                rectptr->y = y << mbit;
                placed.push_back(rectptr);
                return true;
            }
        }
        if (skiplines) {
            y += rh - vstep;
        }
    }
    return false;
}

