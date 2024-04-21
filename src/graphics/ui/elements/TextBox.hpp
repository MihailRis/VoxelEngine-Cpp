#ifndef GRAPHICS_UI_ELEMENTS_TEXTBOX_HPP_
#define GRAPHICS_UI_ELEMENTS_TEXTBOX_HPP_

#include "Panel.hpp"
#include "Label.hpp"

class Font;

namespace gui {
    class Label;
    
    class TextBox : public Panel {
    protected:
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 invalidColor {0.1f, 0.05f, 0.03f, 1.0f};
        std::shared_ptr<Label> label;
        std::wstring input;
        std::wstring placeholder;
        wstringsupplier supplier = nullptr;
        wstringconsumer consumer = nullptr;
        wstringchecker validator = nullptr;
        runnable onEditStart = nullptr;
        bool valid = true;
        /// @brief text input pointer, value may be greather than text length
        uint caret = 0;
        /// @brief actual local (line) position of the caret on vertical move
        uint maxLocalCaret = 0;
        uint textOffset = 0;
        int textInitX;
        /// @brief last time of the caret was moved (used for blink animation)
        double caretLastMove = 0.0;
        Font* font = nullptr;

        size_t selectionStart = 0;
        size_t selectionEnd = 0;
        size_t selectionOrigin = 0;

        bool multiline = false;
        bool editable = true;

        size_t normalizeIndex(int index);

        int calcIndexAt(int x, int y) const;
        void paste(const std::wstring& text);
        void setTextOffset(uint x);
        void erase(size_t start, size_t length);
        bool eraseSelected();
        void resetSelection();
        void extendSelection(int index);
        size_t getLineLength(uint line) const;

        /// @brief Get total length of the selection 
        size_t getSelectionLength() const;

        /// @brief Set maxLocalCaret to local (line) caret position
        void resetMaxLocalCaret();

        void performEditingKeyboardEvents(keycode key);
    public:
        TextBox(
            std::wstring placeholder, 
            glm::vec4 padding=glm::vec4(4.0f)
        );

        virtual void setTextSupplier(wstringsupplier supplier);

        /// @brief Consumer called on stop editing text (textbox defocus)
        /// @param consumer std::wstring consumer function
        virtual void setTextConsumer(wstringconsumer consumer);

        /// @brief Text validator called while text editing and returns true if
        /// text is valid
        /// @param validator std::wstring consumer returning boolean 
        virtual void setTextValidator(wstringchecker validator);

        virtual void setFocusedColor(glm::vec4 color);
        virtual glm::vec4 getFocusedColor() const;

        /// @brief Set color of textbox marked by validator as invalid
        virtual void setErrorColor(glm::vec4 color);

        /// @brief Get color of textbox marked by validator as invalid
        virtual glm::vec4 getErrorColor() const;
        
        /// @brief Get TextBox content text or placeholder if empty
        virtual std::wstring getText() const;

        /// @brief Set TextBox content text
        virtual void setText(std::wstring value);

        /// @brief Get text placeholder
        virtual std::wstring getPlaceholder() const;

        /// @brief Set text placeholder
        /// @param text will be used instead of empty
        virtual void setPlaceholder(const std::wstring& text);
        
        /// @brief Get selected text
        virtual std::wstring getSelection() const;

        /// @brief Get current caret position in text
        /// @return integer in range [0, text.length()]
        virtual uint getCaret() const;

        /// @brief Set caret position in the text
        /// @param position integer in range [0, text.length()]
        virtual void setCaret(uint position);

        /// @brief Select part of the text
        /// @param start index of the first selected character
        /// @param end index of the last selected character + 1
        virtual void select(int start, int end);

        /// @brief Check text with validator set with setTextValidator
        /// @return true if text is valid
        virtual bool validate();

        virtual void setValid(bool valid);
        virtual bool isValid() const;

        /// @brief Enable/disable multiline mode        
        virtual void setMultiline(bool multiline);

        /// @brief Check if multiline mode is enabled 
        virtual bool isMultiline() const;

        /// @brief Enable/disable text editing feature
        virtual void setEditable(bool editable);

        /// @brief Check if text editing feature is enabled 
        virtual bool isEditable() const;

        /// @brief Set runnable called on textbox focus
        virtual void setOnEditStart(runnable oneditstart);

        virtual void onFocus(GUI*) override;
        virtual void refresh() override;
        virtual void click(GUI*, int, int) override;
        virtual void mouseMove(GUI*, int x, int y) override;
        virtual bool isFocuskeeper() const override {return true;}
        virtual void draw(const GfxContext* pctx, Assets* assets) override;
        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;
        virtual void typed(unsigned int codepoint) override; 
        virtual void keyPressed(keycode key) override;
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_TEXTBOX_HPP_
