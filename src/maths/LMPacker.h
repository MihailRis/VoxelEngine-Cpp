/*
    C++ LMPacker port
        https://github.com/MihailRis/LMPacker
*/
#ifndef LMPACKER_H_
#define LMPACKER_H_

#include <stdlib.h>
#include <stdint.h>
#include <vector>

struct rectangle {
	int x;
	int y;
	int width;
	int height;
	int extX = 0;
	int extY = 0;
	uint idx;

	rectangle(uint idx, int x, int y, int width, int height)
		: idx(idx), x(x), y(y), width(width), height(height){
	}
};

class LMPacker {
	std::vector<rectangle> rects;
	std::vector<rectangle*> placed;
	uint32_t width = 0;
	uint32_t height = 0;
	rectangle*** matrix = nullptr;
	uint32_t mbit = 0;

	void cleanup();
	bool place(rectangle* rect, uint32_t vstep);
public:
	LMPacker(const uint32_t sizes[], size_t length);
	virtual ~LMPacker();

	bool buildCompact(uint32_t width, uint32_t height, uint16_t extension) {
		return build(width, height, extension, 0, 1);
	}
	bool buildFast(uint32_t width, uint32_t height, uint16_t extension) {
		return build(width, height, extension, 1, 2);
	}
	bool build(uint32_t width, uint32_t height, uint16_t extension, uint32_t mbit, uint32_t vstep);

	std::vector<rectangle> getResult() {
		return rects;
	}
};

#endif /* LMPACKER_H_ */