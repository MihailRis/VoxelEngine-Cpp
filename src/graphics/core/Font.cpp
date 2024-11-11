#include "Font.hpp"

#include <utility>
#include "Texture.hpp"
#include "Batch2D.hpp"

inline constexpr uint GLYPH_SIZE = 16;
inline constexpr uint MAX_CODEPAGES = 10000; // idk ho many codepages unicode has
inline constexpr glm::vec4 SHADOW_TINT(0.0f, 0.0f, 0.0f, 1.0f);

Font::Font(std::vector<std::unique_ptr<Texture>> pages, int lineHeight, int yoffset) 
    : lineHeight(lineHeight), yoffset(yoffset), pages(std::move(pages)) {
}

Font::~Font() = default;

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

int Font::calcWidth(const std::wstring& text, size_t length) {
    return calcWidth(text, 0, length);
}

int Font::calcWidth(const std::wstring& text, size_t offset, size_t length) {
    return std::min(text.length()-offset, length) * glyphInterval;
}

static inline void drawGlyph(
    Batch2D* batch, int x, int y, uint c, int glyphSize
) {
    batch->sprite(x, y, glyphSize, glyphSize, 16, c, batch->getColor());
}

void Font::draw(Batch2D* batch, std::wstring_view text, int x, int y) {
    uint page = 0;
    uint next = MAX_CODEPAGES;
    int init_x = x;
    do {
        for (uint c : text){
            if (!isPrintableChar(c)) {
                x += glyphInterval;
                continue;
            }
            uint charpage = c >> 8;
            if (charpage == page){
                Texture* texture = nullptr;
                if (charpage < pages.size()) {
                    texture = pages[charpage].get();
                }
                if (texture == nullptr){
                    texture = pages[0].get();
                }
                batch->texture(texture);
                drawGlyph(batch, x, y, c, lineHeight);
            }
            else if (charpage > page && charpage < next){
                next = charpage;
            }
            x += glyphInterval;
        }
        page = next;
        next = MAX_CODEPAGES;
        x = init_x;
    } while (page < MAX_CODEPAGES);
}
