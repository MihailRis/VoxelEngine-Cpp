#ifndef GRAPHICS_CORE_FONT_HPP_
#define GRAPHICS_CORE_FONT_HPP_

#include <memory>
#include <string>
#include <vector>
#include "typedefs.hpp"

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
    /// @param length max substring length (default: no limit)
    /// @return pixel width of the substring 
    int calcWidth(const std::wstring& text, size_t length=-1);

    /// @brief Calculate text width in pixels
    /// @param text selected text
    /// @param offset start of the substring
    /// @param length max substring length
    /// @return pixel width of the substring
    int calcWidth(const std::wstring& text, size_t offset, size_t length);

    /// @brief Check if character is visible (non-whitespace)
    /// @param codepoint character unicode codepoint
    bool isPrintableChar(uint codepoint) const;
    void draw(Batch2D* batch, std::wstring text, int x, int y);
    void draw(Batch2D* batch, const std::wstring& text, int x, int y, FontStyle style);
    void draw(Batch2D* batch, std::wstring_view text, int x, int y, FontStyle style);
};

#endif // GRAPHICS_CORE_FONT_HPP_
