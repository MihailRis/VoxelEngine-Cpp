#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "typedefs.hpp"

class Texture;
class Batch2D;
class Batch3D;
class Camera;

struct FontStyle {
    bool bold = false;
    bool italic = false;
    glm::vec4 color {1, 1, 1, 1};
};

class Font {
    int lineHeight;
    int yoffset;
    int glyphInterval = 8;
    std::vector<std::unique_ptr<Texture>> pages;
public:
    Font(std::vector<std::unique_ptr<Texture>> pages, int lineHeight, int yoffset);
    ~Font();

    int getLineHeight() const;
    int getYOffset() const;
    
    /// @brief Calculate text width in pixels
    /// @param text selected text
    /// @param length max substring length (default: no limit)
    /// @return pixel width of the substring 
    int calcWidth(const std::wstring& text, size_t length=-1) const;

    /// @brief Calculate text width in pixels
    /// @param text selected text
    /// @param offset start of the substring
    /// @param length max substring length
    /// @return pixel width of the substring
    int calcWidth(const std::wstring& text, size_t offset, size_t length) const;

    /// @brief Check if character is visible (non-whitespace)
    /// @param codepoint character unicode codepoint
    bool isPrintableChar(uint codepoint) const;
    
    void draw(Batch2D& batch, std::wstring_view text, int x, int y, float scale=1) const;

    void draw(
        Batch3D& batch,
        std::wstring_view text,
        const glm::vec3& pos,
        const glm::vec3& right={1, 0, 0},
        const glm::vec3& up={0, 1, 0}
    ) const;

    const Texture* getPage(int page) const;
};
