#include "Font.h"
#include "Texture.h"
#include "Batch2D.h"

inline constexpr uint GLYPH_SIZE = 16;
inline constexpr uint MAX_CODEPAGES = 10000; // idk ho many codepages unicode has
inline constexpr glm::vec4 SHADOW_TINT(0.0f, 0.0f, 0.0f, 1.0f);

Font::Font(std::vector<std::unique_ptr<Texture>> pages, int lineHeight, int yoffset) 
    : lineHeight(lineHeight), yoffset(yoffset), pages(std::move(pages)) {
}

Font::~Font(){
}

int Font::getYOffset() const {
    return yoffset;
}

int Font::getLineHeight() const {
    return lineHeight;
}

bool Font::isPrintableChar(uint codepoint) const {
    switch (codepoint){
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

int Font::calcWidth(std::wstring text, size_t length) {
    return std::min(text.length(), length) * 8;
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y) {
    draw(batch, text, x, y, FontStyle::none);
}

static inline void drawGlyph(Batch2D* batch, int x, int y, uint c, FontStyle style) {
    switch (style){
        case FontStyle::shadow:
            batch->sprite(x+1, y+1, GLYPH_SIZE, GLYPH_SIZE, 16, c, SHADOW_TINT);
            break;
        case FontStyle::outline:
            for (int oy = -1; oy <= 1; oy++){
                for (int ox = -1; ox <= 1; ox++){
                    if (ox || oy) {
                        batch->sprite(x+ox, y+oy, GLYPH_SIZE, GLYPH_SIZE, 16, c, SHADOW_TINT);
                    }
                }
            }
            break;
    }
    batch->sprite(x, y, GLYPH_SIZE, GLYPH_SIZE, 16, c, batch->color);
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y, FontStyle style) {
    draw(batch, std::wstring_view(text.c_str(), text.length()), x, y, style);
}

void Font::draw(Batch2D* batch, std::wstring_view text, int x, int y, FontStyle style) {
    uint page = 0;
    uint next = MAX_CODEPAGES;
    int init_x = x;
    do {
        for (uint c : text){
            if (!isPrintableChar(c)) {
                x += 8;
                continue;
            }
            int charpage = c >> 8;
            if (charpage == page){
                Texture* texture = pages[charpage].get();
                if (texture == nullptr){
                    texture = pages[0].get();
                }
                batch->texture(texture);
                drawGlyph(batch, x, y, c, style);
            }
            else if (charpage > page && charpage < next){
                next = charpage;
            }
            x += 8;//getGlyphWidth(c);
        }
        page = next;
        next = MAX_CODEPAGES;
        x = init_x;
    } while (page < MAX_CODEPAGES);
}
