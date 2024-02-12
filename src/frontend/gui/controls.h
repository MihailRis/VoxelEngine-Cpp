#ifndef FRONTEND_GUI_CONTROLS_H_
#define FRONTEND_GUI_CONTROLS_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "GUI.h"
#include "UINode.h"
#include "panels.h"
#include "../../window/input.h"
#include "../../delegates.h"

class Batch2D;
class Assets;

namespace gui {
    using wstringsupplier = std::function<std::wstring()>;
    using wstringconsumer = std::function<void(std::wstring)>;

    using doublesupplier = std::function<double()>;
    using doubleconsumer = std::function<void(double)>;

    using boolsupplier = std::function<bool()>;
    using boolconsumer = std::function<void(bool)>;

    using wstringchecker = std::function<bool(const std::wstring&)>;

    class Label : public UINode {
    protected:
        std::wstring text;
        std::string fontName_;
        wstringsupplier supplier = nullptr;
    public:
        Label(std::string text, std::string fontName="normal");
        Label(std::wstring text, std::string fontName="normal");

        virtual void setText(std::wstring text);
        std::wstring getText() const;

        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual Label* textSupplier(wstringsupplier supplier);
    };

    class Image : public UINode {
    protected:
        std::string texture;
    public:
        Image(std::string texture, glm::vec2 size=glm::vec2(32,32));

        virtual void draw(const GfxContext* pctx, Assets* assets) override;
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
        virtual void textSupplier(wstringsupplier supplier);
        virtual void textConsumer(wstringconsumer consumer);
        virtual void textValidator(wstringchecker validator);
        virtual bool isFocuskeeper() const override {return true;}
        virtual std::wstring getText() const;
        virtual void setText(std::wstring value);
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
        glm::vec4 hoverColor {0.01f, 0.02f, 0.03f, 0.5f};
        glm::vec4 trackColor {1.0f, 1.0f, 1.0f, 0.4f};
        doublesupplier supplier_ = nullptr;
        doubleconsumer consumer_ = nullptr;
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

        virtual void supplier(doublesupplier supplier);
        virtual void consumer(doubleconsumer consumer);

        virtual void mouseMove(GUI*, int x, int y) override;
    };

    class CheckBox : public UINode {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 checkColor {1.0f, 1.0f, 1.0f, 0.4f};
        boolsupplier supplier_ = nullptr;
        boolconsumer consumer_ = nullptr;
        bool checked_ = false;
    public:
        CheckBox(bool checked=false);

        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual void mouseRelease(GUI*, int x, int y) override;

        virtual void supplier(boolsupplier supplier);
        virtual void consumer(boolconsumer consumer);

        virtual CheckBox* checked(bool flag);

        virtual bool checked() const {
            if (supplier_)
                return supplier_();
            return checked_;
        }
    };

    class FullCheckBox : public Panel {
    protected:
        std::shared_ptr<CheckBox> checkbox;
    public:
        FullCheckBox(std::wstring text, glm::vec2 size, bool checked=false);

        virtual void supplier(boolsupplier supplier) {
            checkbox->supplier(supplier);
        }

        virtual void consumer(boolconsumer consumer) {
            checkbox->consumer(consumer);
        }

        virtual void checked(bool flag) {
            checkbox->checked(flag);
        }

        virtual bool checked() const {
            return checkbox->checked();
        }
    };
}

#endif // FRONTEND_GUI_CONTROLS_H_