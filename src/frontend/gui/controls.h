#ifndef FRONTEND_GUI_CONTROLS_H_
#define FRONTEND_GUI_CONTROLS_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "GUI.h"
#include "UINode.h"
#include "containers.h"
#include "../../window/input.h"
#include "../../delegates.h"

class Batch2D;
class Assets;

namespace gui {
    class Label : public UINode {
    protected:
        std::wstring text;
        std::string fontName;
        wstringsupplier supplier = nullptr;
    public:
        Label(std::string text, std::string fontName="normal");
        Label(std::wstring text, std::string fontName="normal");

        virtual void setText(std::wstring text);
        std::wstring getText() const;

        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual void textSupplier(wstringsupplier supplier);
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
    public:
        TextBox(std::wstring placeholder, 
                glm::vec4 padding=glm::vec4(4.0f));

        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;
        virtual void typed(unsigned int codepoint) override; 
        virtual void keyPressed(int key) override;
        virtual void setTextSupplier(wstringsupplier supplier);
        virtual void setTextConsumer(wstringconsumer consumer);
        virtual void setTextValidator(wstringchecker validator);
        virtual bool isFocuskeeper() const override {return true;}
        virtual void setFocusedColor(glm::vec4 color);
        virtual glm::vec4 getFocusedColor() const;
        virtual void setErrorColor(glm::vec4 color);
        virtual glm::vec4 getErrorColor() const;
        /* Get TextBox content text or placeholder if empty */
        virtual std::wstring getText() const;
        /* Set TextBox content text */
        virtual void setText(std::wstring value);
        virtual std::wstring getPlaceholder() const;
        virtual void setPlaceholder(const std::wstring&);
        virtual bool validate();
        virtual void setValid(bool valid);
        virtual bool isValid() const;
        virtual void setOnEditStart(runnable oneditstart);
        virtual void focus(GUI*) override;
        virtual void refresh() override;
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

        virtual void clicked(GUI*, int button) override;
        virtual void keyPressed(int key) override;
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

#endif // FRONTEND_GUI_CONTROLS_H_
