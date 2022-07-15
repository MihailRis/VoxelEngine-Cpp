#ifndef GRAPHICS_FONT_H_
#define GRAPHICS_FONT_H_

#include <string>
#include <vector>

class Texture;
class Batch2D;

class Font {
public:
	std::vector<Texture*> pages;
	Font(std::vector<Texture*> pages);
	~Font();

	int getGlyphWidth(char c);
	bool isPrintableChar(int c);
	void draw(Batch2D* batch, std::wstring text, int x, int y);
	void drawWithOutline(Batch2D* batch, std::wstring text, int x, int y);
};

#endif /* GRAPHICS_FONT_H_ */
