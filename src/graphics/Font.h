#ifndef GRAPHICS_FONT_H_
#define GRAPHICS_FONT_H_

#include <string>

class Texture;
class Batch2D;

class Font {
public:
	Texture* texture;
	Font(Texture* texture);
	~Font();

	int getGlyphWidth(char c);
	bool isPrintableChar(char c);
	void draw(Batch2D* batch, std::string text, int x, int y);
	void drawWithShadow(Batch2D* batch, std::string text, int x, int y);
};

#endif /* GRAPHICS_FONT_H_ */
