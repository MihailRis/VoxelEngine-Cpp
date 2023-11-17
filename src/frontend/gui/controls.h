#ifndef FRONTEND_GUI_CONTROLS_H_
#define FRONTEND_GUI_CONTROLS_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "UINode.h"
#include "panels.h"

class Batch2D;
class Assets;

namespace gui {
    typedef std::function<std::wstring()> wstringsupplier;
    typedef std::function<void(std::wstring)> wstringconsumer;

    class Label : public UINode {
    protected:
        std::wstring text_;
        std::string fontName_;
        wstringsupplier supplier = nullptr;
    public:
        Label(std::wstring text, std::string fontName="normal");

        virtual Label& text(std::wstring text);
        std::wstring text() const;

        virtual void draw(Batch2D* batch, Assets* assets);

        virtual void textSupplier(wstringsupplier supplier);
    };
    class Button : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 pressedColor {0.0f, 0.0f, 0.0f, 0.95f};
        std::vector<onaction> actions;
    public:
        Button(std::shared_ptr<UINode> content, glm::vec4 padding=glm::vec4(2.0f));
        Button(std::wstring text, glm::vec4 padding=glm::vec4(2.0f));

        virtual void drawBackground(Batch2D* batch, Assets* assets);

        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        virtual void mouseRelease(GUI*, int x, int y) override;
        virtual Button* listenAction(onaction action);
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
        TextBox(std::wstring placeholder, glm::vec4 padding=glm::vec4(2.0f));

        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        virtual void drawBackground(Batch2D* batch, Assets* assets) override;
        virtual void typed(unsigned int codepoint) override; 
        virtual void keyPressed(int key) override;
        virtual void textSupplier(wstringsupplier supplier);
        virtual void textConsumer(wstringconsumer consumer);
        virtual bool isfocuskeeper() const override {return true;}
        virtual std::wstring text() const;
    };
}

#endif // FRONTEND_GUI_CONTROLS_H_