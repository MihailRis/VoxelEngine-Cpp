#include "Font.h"
#include "Texture.h"
#include "Batch2D.h"

Font::Font(Texture* texture) : texture(texture) {
}

Font::~Font(){
	delete texture;
}


void Font::draw(Batch2D* batch, std::string text, int x, int y) {
	for (char c : text){
		float u = (c % 16) / 16.0f;
		float v = 1.0f - ((c / 16) / 16.0f) - 1.0f/16.0f;
		batch->rect(x, y, 8, 8, u, v, 1.0f/16.0f, 1.0f/16.0f, 1,1,1,1);

		int gw = 7;
		switch (c){
		case 'l':
		case 'i':
		case 'j':
		case '|':
		case '.':
		case ',':
		case ':':
		case ';':
			gw = 3;
			break;
		case 't':
			gw = 5;
			break;
		case ' ':
			gw = 3;
			break;
		}
		x += gw;
	}
}
