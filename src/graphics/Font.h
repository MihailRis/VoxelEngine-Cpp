#ifndef GRAPHICS_FONT_H_
#define GRAPHICS_FONT_H_

#include <string>
#include <vector>

namespace vulkan {
	class Batch2D;
}

class ITexture;
class Batch2D;

#define STYLE_NONE 0
#define STYLE_SHADOW 1
#define STYLE_OUTLINE 2

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
