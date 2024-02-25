#ifndef GRAPHICS_FONT_H_
#define GRAPHICS_FONT_H_

#include <memory>
#include <string>
#include <vector>
#include "../typedefs.h"

class Texture;
class Batch2D;

enum class FontStyle {
    none,
    shadow,
    outline
};

class Font {
    int lineHeight;
    int yoffset;
public:
    std::vector<std::unique_ptr<Texture>> pages;
    Font(std::vector<std::unique_ptr<Texture>> pages, int lineHeight, int yoffset);
    ~Font();

    int getLineHeight() const;
    int getYOffset() const;
    
    /// @brief Calculate text width in pixels
    /// @param text selected text
    /// @param length max text chunk length (default: no limit)
    /// @return pixel width of the text 
    int calcWidth(std::wstring text, size_t length=-1);

    /// @brief Check if character is visible (non-whitespace)
    /// @param codepoint character unicode codepoint
    bool isPrintableChar(uint codepoint) const;
    void draw(Batch2D* batch, std::wstring text, int x, int y);
    void draw(Batch2D* batch, std::wstring text, int x, int y, FontStyle style);
    void draw(Batch2D* batch, std::wstring_view text, int x, int y, FontStyle style);
};

#endif /* GRAPHICS_FONT_H_ */
