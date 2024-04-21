#ifndef GRAPHICS_UI_ELEMENTS_LABEL_HPP_
#define GRAPHICS_UI_ELEMENTS_LABEL_HPP_

#include "UINode.hpp"

namespace gui {
    class Label : public UINode {
    protected:
        std::wstring text;
        std::string fontName;
        wstringsupplier supplier = nullptr;
        uint lines = 1;
        float lineInterval = 1.5f;
        Align valign = Align::center;

        bool multiline = false;

        // runtime values
        
        /// @brief Text Y offset relative to label position
        /// (last calculated alignment)
        int textYOffset = 0;

        /// @brief Text line height multiplied by line interval
        int totalLineHeight = 1;
    public:
        Label(std::string text, std::string fontName="normal");
        Label(std::wstring text, std::string fontName="normal");

        virtual void setText(std::wstring text);
        const std::wstring& getText() const;

        virtual void setFontName(std::string name);
        virtual const std::string& getFontName() const;

        /// @brief Set text vertical alignment (default value: center)
        /// @param align Align::top / Align::center / Align::bottom
        virtual void setVerticalAlign(Align align);
        virtual Align getVerticalAlign() const;

        /// @brief Get line height multiplier used for multiline labels 
        /// (default value: 1.5)
        virtual float getLineInterval() const;

        /// @brief Set line height multiplier used for multiline labels
        virtual void setLineInterval(float interval);

        /// @brief Get Y position of the text relative to label position
        /// @return Y offset
        virtual int getTextYOffset() const;

        /// @brief Get Y position of the line relative to label position
        /// @param line target line index
        /// @return Y offset
        virtual int getLineYOffset(uint line) const;

        /// @brief Get position of line start in the text
        /// @param line target line index
        /// @return position in the text [0..length]
        virtual size_t getTextLineOffset(uint line) const;

        /// @brief Get line index by its Y offset relative to label position
        /// @param offset target Y offset
        /// @return line index [0..+]
        virtual uint getLineByYOffset(int offset) const;
        virtual uint getLineByTextIndex(size_t index) const;
        virtual uint getLinesNumber() const;

        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual void textSupplier(wstringsupplier supplier);

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_LABEL_HPP_
