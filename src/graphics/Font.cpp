#include "Font.h"
#include "Texture.h"
#include "Batch2D.h"

Font::Font(std::vector<Texture*> pages) : pages(pages) {
}

Font::~Font(){
	for (Texture* texture : pages)
		delete texture;
}

int Font::getGlyphWidth(char c) {
	switch (c){
		case 'l':
		case 'i':
		case 'j':
		case '|':
		case '.':
		case ',':
		case ':':
		case ';': return 3;
		case 't': return 5;
		case ' ': return 3;
	}
	return 7;
}


bool Font::isPrintableChar(int c) {
	switch (c){
	case ' ':
	case '\t':
	case '\n':
	case '\f':
	case '\r':
		return false;
	default:
		return true;
	}
}

#define RES 16

void Font::draw(Batch2D* batch, std::wstring text, int x, int y) {
	for (unsigned c : text){
		if (isPrintableChar(c)){
			batch->texture(pages[c >> 8]);
			batch->sprite(x, y, RES, RES, 16, c, vec4(1.0f));
		}
		x += getGlyphWidth(c);
	}
}

void Font::drawWithShadow(Batch2D* batch, std::wstring text, int x, int y) {
	for (unsigned c : text){
		if (isPrintableChar(c)){
		    Texture* texture = pages[c >> 8];
		    if (texture == nullptr){
		        texture = pages[0];
		    }
			batch->texture(texture);
			batch->sprite(x+1, y+1, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			batch->sprite(x+1, y-1, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			batch->sprite(x-1, y, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			batch->sprite(x+1, y, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));

			batch->sprite(x-1, y-1, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			batch->sprite(x+1, y-1, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			batch->sprite(x+1, y+1, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			batch->sprite(x-1, y+1, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));

			batch->sprite(x, y, RES, RES, 16, c, vec4(1.0f));
		}
		x += getGlyphWidth(c);
	}
}
