#ifndef FRONTEND_GUI_CONTROLS_H_
#define FRONTEND_GUI_CONTROLS_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "UINode.h"
#include "panels.h"
#include "../../window/input.h"

class Batch2D;
class Assets;

namespace gui {
    typedef std::function<std::wstring()> wstringsupplier;
    typedef std::function<void(std::wstring)> wstringconsumer;

    typedef std::function<double()> doublesupplier;
    typedef std::function<void(double)> doubleconsumer;

    typedef std::function<bool()> boolsupplier;
    typedef std::function<void(bool)> boolconsumer;

    typedef std::function<bool(const std::wstring&)> wstringchecker;

    class Label : public UINode {
    protected:
        std::wstring text_;
        std::string fontName_;
        wstringsupplier supplier = nullptr;
    public:
        Label(std::wstring text, std::string fontName="normal");

        virtual Label& text(std::wstring text);
        std::wstring text() const;

        virtual void draw(Batch2D* batch, Assets* assets) override;

        virtual Label* textSupplier(wstringsupplier supplier);
        virtual void size(glm::vec2 size) override;
    };

    class Button : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.15f, 0.75f};
        glm::vec4 pressedColor {0.0f, 0.0f, 0.0f, 0.95f};
        std::vector<onaction> actions;
        std::shared_ptr<UINode> label = nullptr;
    public:
        Button(std::shared_ptr<UINode> content, glm::vec4 padding=glm::vec4(2.0f));
        Button(std::wstring text, 
               glm::vec4 padding=glm::vec4(2.0f), 
               glm::vec4 margin=glm::vec4(1.0f));

        virtual void drawBackground(Batch2D* batch, Assets* assets);

        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        virtual void mouseRelease(GUI*, int x, int y) override;
        virtual Button* listenAction(onaction action);

        virtual void textAlign(Align align);

        virtual void text(std::wstring text);
        virtual std::wstring text() const;

        virtual Button* textSupplier(wstringsupplier supplier);
    };

    class TextBox : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 invalidColor {0.1f, 0.05f, 0.03f, 1.0f};
        Label* label;
        std::wstring input;
        std::wstring placeholder;
        wstringsupplier supplier = nullptr;
        wstringconsumer consumer = nullptr;
        wstringchecker validator = nullptr;
        bool valid = true;
    public:
        TextBox(std::wstring placeholder, 
                glm::vec4 padding=glm::vec4(2.0f));

        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        virtual void drawBackground(Batch2D* batch, Assets* assets) override;
        virtual void typed(unsigned int codepoint) override; 
        virtual void keyPressed(int key) override;
        virtual void textSupplier(wstringsupplier supplier);
        virtual void textConsumer(wstringconsumer consumer);
        virtual void textValidator(wstringchecker validator);
        virtual bool isfocuskeeper() const override {return true;}
        virtual std::wstring text() const;
        virtual bool validate();
        virtual void setValid(bool valid);
        virtual bool isValid() const;
    };

    class InputBindBox : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        Label* label;
        Binding& binding;
    public:
        InputBindBox(Binding& binding, glm::vec4 padding=glm::vec4(6.0f));
        virtual void drawBackground(Batch2D* batch, Assets* assets) override;
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        virtual void clicked(GUI*, int button) override;
        virtual void keyPressed(int key) override;
        virtual bool isfocuskeeper() const override {return true;}
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
        virtual void draw(Batch2D* batch, Assets* assets) override;

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

        virtual void draw(Batch2D* batch, Assets* assets) override;

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