#ifndef GRAPHICS_FONT_H_
#define GRAPHICS_FONT_H_

#include <string>
#include <vector>
#include "../typedefs.h"

#ifdef USE_VULKAN
namespace vulkan {
	class Batch2D;
}
#endif

class ITexture;
class Batch2D;

const uint STYLE_NONE = 0;
const uint STYLE_SHADOW = 1;
const uint STYLE_OUTLINE = 2;

class Font {
	int lineHeight_;
public:
	std::vector<ITexture*> pages;
	Font(std::vector<ITexture*> pages, int lineHeight);
	~Font();

	int lineHeight() const;
	int calcWidth(std::wstring text);
	// int getGlyphWidth(char c);
	bool isPrintableChar(int c);
	void draw(vulkan::Batch2D* batch, std::wstring text, int x, int y);
	void draw(vulkan::Batch2D* batch, std::wstring text, int x, int y, int style);
};

#endif /* GRAPHICS_FONT_H_ */
