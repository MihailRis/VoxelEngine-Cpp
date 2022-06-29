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


void Font::draw(Batch2D* batch, std::string text, int x, int y) {
	for (char c : text){
		float u = (c % 16) / 16.0f;
		float v = 1.0f - ((c / 16) / 16.0f) - 1.0f/16.0f;
		batch->rect(x, y, 8, 8, u, v, 1.0f/16.0f, 1.0f/16.0f, 1,1,1,1);
		x += getGlyphWidth(c);
	}
}
