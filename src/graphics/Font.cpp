#include "Font.h"
#include "Texture.h"
#include "Batch2D.h"

using glm::vec4;

Font::Font(std::vector<Texture*> pages, int lineHeight) : lineHeight_(lineHeight), pages(pages) {
}

Font::~Font(){
	for (Texture* texture : pages)
		delete texture;
}

int Font::lineHeight() const {
	return lineHeight_;
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

const int RES = 16;

int Font::calcWidth(std::wstring text) {
	return text.length() * 8;
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y) {
	draw(batch, text, x, y, STYLE_NONE);
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y, int style) {
	int page = 0;
	int next = 10000;
	int init_x = x;
	do {
		for (unsigned c : text){
			if (isPrintableChar(c)){
				int charpage = c >> 8;
				if (charpage == page){
				    Texture* texture = pages[charpage];
				    if (texture == nullptr){
				        texture = pages[0];
				    }
					batch->texture(pages[charpage]);

					switch (style){
						case STYLE_SHADOW:
							batch->sprite(x+1, y+1, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
							break;
						case STYLE_OUTLINE:
							for (int oy = -1; oy <= 1; oy++){
								for (int ox = -1; ox <= 1; ox++){
									if (ox || oy)
										batch->sprite(x+ox, y+oy, RES, RES, 16, c, vec4(0.0f, 0.0f, 0.0f, 1.0f));
								}
							}
							break;
					}

					batch->sprite(x, y, RES, RES, 16, c, batch->color);
				}
				else if (charpage > page && charpage < next){
					next = charpage;
				}
			}
			x += 8;//getGlyphWidth(c);
		}
		page = next;
		next = 10000;
		x = init_x;
	} while (page < 10000);
}
