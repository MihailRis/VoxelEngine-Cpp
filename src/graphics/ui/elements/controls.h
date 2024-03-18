#ifndef GRAPHICS_UI_ELEMENTS_CONTROLS_H_
#define GRAPHICS_UI_ELEMENTS_CONTROLS_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "../GUI.h"
#include "UINode.h"
#include "containers.h"
#include "../../../window/input.h"
#include "../../../delegates.h"
#include "../../../typedefs.h"

class Batch2D;
class Assets;
class Font;

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

    class Image : public UINode {
    protected:
        std::string texture;
        bool autoresize = false;
    public:
        Image(std::string texture, glm::vec2 size=glm::vec2(32,32));

        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual void setAutoResize(bool flag);
        virtual bool isAutoResize() const;
    };

    class Button : public Panel {
    protected:
        glm::vec4 pressedColor {0.0f, 0.0f, 0.0f, 0.95f};
        std::vector<onaction> actions;
        std::shared_ptr<Label> label = nullptr;
    public:
        Button(std::shared_ptr<UINode> content, 
               glm::vec4 padding=glm::vec4(2.0f));
               
        Button(std::wstring text, 
               glm::vec4 padding,
               onaction action,
               glm::vec2 size=glm::vec2(-1));

        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;

        virtual void mouseRelease(GUI*, int x, int y) override;
        virtual Button* listenAction(onaction action);

        virtual Align getTextAlign() const;
        virtual void setTextAlign(Align align);

        virtual void setText(std::wstring text);
        virtual std::wstring getText() const;

        virtual glm::vec4 getPressedColor() const;
        virtual void setPressedColor(glm::vec4 color);

        virtual Button* textSupplier(wstringsupplier supplier);

        virtual void refresh() override;
    };

    class RichButton : public Container {
    protected:
        glm::vec4 pressedColor {0.0f, 0.0f, 0.0f, 0.95f};
        std::vector<onaction> actions;
    public:
        RichButton(glm::vec2 size);

        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;

        virtual void mouseRelease(GUI*, int x, int y) override;
        virtual RichButton* listenAction(onaction action);
    };

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

    class InputBindBox : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        std::shared_ptr<Label> label;
        Binding& binding;
    public:
        InputBindBox(Binding& binding, glm::vec4 padding=glm::vec4(6.0f));
        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;

        virtual void clicked(GUI*, mousecode button) override;
        virtual void keyPressed(keycode key) override;
        virtual bool isFocuskeeper() const override {return true;}
    };

    class TrackBar : public UINode {
    protected:
        glm::vec4 trackColor {1.0f, 1.0f, 1.0f, 0.4f};
        doublesupplier supplier = nullptr;
        doubleconsumer consumer = nullptr;
        double min;
        double max;
        double value;
        double step;
        int trackWidth;
    public:
        TrackBar(double min, 
                 double max, 
                 double value, 
                 double step=1.0, 
                 int trackWidth=1);
        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual void setSupplier(doublesupplier supplier);
        virtual void setConsumer(doubleconsumer consumer);

        virtual void mouseMove(GUI*, int x, int y) override;

        virtual double getValue() const;
        virtual double getMin() const;
        virtual double getMax() const;
        virtual double getStep() const;
        virtual int getTrackWidth() const;
        virtual glm::vec4 getTrackColor() const;

        virtual void setValue(double);
        virtual void setMin(double);
        virtual void setMax(double);
        virtual void setStep(double);
        virtual void setTrackWidth(int);
        virtual void setTrackColor(glm::vec4);
    };

    class CheckBox : public UINode {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 checkColor {1.0f, 1.0f, 1.0f, 0.4f};
        boolsupplier supplier = nullptr;
        boolconsumer consumer = nullptr;
        bool checked = false;
    public:
        CheckBox(bool checked=false);

        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual void mouseRelease(GUI*, int x, int y) override;

        virtual void setSupplier(boolsupplier supplier);
        virtual void setConsumer(boolconsumer consumer);

        virtual CheckBox* setChecked(bool flag);

        virtual bool isChecked() const {
            if (supplier)
                return supplier();
            return checked;
        }
    };

    class FullCheckBox : public Panel {
    protected:
        std::shared_ptr<CheckBox> checkbox;
    public:
        FullCheckBox(std::wstring text, glm::vec2 size, bool checked=false);

        virtual void setSupplier(boolsupplier supplier) {
            checkbox->setSupplier(supplier);
        }

        virtual void setConsumer(boolconsumer consumer) {
            checkbox->setConsumer(consumer);
        }

        virtual void setChecked(bool flag) {
            checkbox->setChecked(flag);
        }

        virtual bool isChecked() const {
            return checkbox->isChecked();
        }
    };
}

#endif // GRAPHICS_UI_ELEMENTS_CONTROLS_H_
