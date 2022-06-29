#include "Font.h"
#include "Texture.h"
#include "Batch2D.h"

Font::Font(Texture* texture) : texture(texture) {
}

Font::~Font(){
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


bool Font::isPrintableChar(char c) {
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


void Font::draw(Batch2D* batch, std::string text, int x, int y) {
	for (char c : text){
		if (isPrintableChar(c))
			batch->sprite(x, y, 8, 8, 16, c, vec4(1.0f));
		x += getGlyphWidth(c);
	}
}

void Font::drawWithShadow(Batch2D* batch, std::string text, int x, int y) {
	for (char c : text){
		if (isPrintableChar(c)){
			batch->sprite(x+1, y+1, 8, 8, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			batch->sprite(x, y, 8, 8, 16, c, vec4(1.0f));
		}
		x += getGlyphWidth(c);
	}
}
