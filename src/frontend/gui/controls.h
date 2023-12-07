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

    class Label : public UINode {
    protected:
        std::wstring text_;
        std::string fontName_;
        wstringsupplier supplier = nullptr;
    public:
        Label(std::wstring text, std::string fontName="normal");

        virtual Label& setText(std::wstring text);
        const std::wstring& text() const;

        void draw(Batch2D* batch, Assets* assets) override;

        virtual Label* setTextSupplier(wstringsupplier supplier);
        void setSize(glm::vec2 size) override;
    };

    class Button : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.15f, 0.75f};
        glm::vec4 pressedColor {0.0f, 0.0f, 0.0f, 0.95f};
        std::vector<onaction> actions;
        std::shared_ptr<UINode> label = nullptr;
    public:
        Button(std::shared_ptr<UINode> content, glm::vec4 padding=glm::vec4(2.0f));
        Button(std::wstring text, glm::vec4 padding=glm::vec4(2.0f));

        virtual void drawBackground(Batch2D* batch, Assets* assets);

        std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        void mouseRelease(GUI*, int x, int y) override;
        virtual Button* listenAction(onaction action);

        virtual void setText(std::wstring text);
        virtual const std::wstring& text() const;
    };

    class TextBox : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        Label* label;
        std::wstring input;
        std::wstring placeholder;
        wstringsupplier supplier = nullptr;
        wstringconsumer consumer = nullptr;
    public:
        TextBox(std::wstring placeholder, 
                glm::vec4 padding=glm::vec4(2.0f));

        std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        void drawBackground(Batch2D* batch, Assets* assets) override;
        void typed(unsigned int codepoint) override; 
        void keyPressed(int key) override;
        virtual void setTextSupplier(wstringsupplier supplier);
        virtual void setTextConsumer(wstringconsumer consumer);
        bool isfocuskeeper() const override {return true;}
        virtual const std::wstring& text() const;
    };

    class InputBindBox : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        Label* label;
        Binding& binding;
    public:
        InputBindBox(Binding& binding, glm::vec4 padding=glm::vec4(6.0f));
        void drawBackground(Batch2D* batch, Assets* assets) override;
        std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        void clicked(GUI*, int button) override;
        void keyPressed(int key) override;
        bool isfocuskeeper() const override {return true;}
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
        void draw(Batch2D* batch, Assets* assets) override;

        virtual void supplier(doublesupplier supplier);
        virtual void consumer(doubleconsumer consumer);

        void mouseMove(GUI*, int x, int y) override;
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

        void draw(Batch2D* batch, Assets* assets) override;

        void mouseRelease(GUI*, int x, int y) override;

        virtual void supplier(boolsupplier supplier);
        virtual void consumer(boolconsumer consumer);

        virtual CheckBox* checked(bool flag);

        virtual bool checked() const {
            if (supplier_)
                return supplier_();
            return checked_;
        }
    };
}

#endif // FRONTEND_GUI_CONTROLS_H_