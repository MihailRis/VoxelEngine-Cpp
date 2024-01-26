#ifndef FRONTEND_GUI_GUI_H_
#define FRONTEND_GUI_GUI_H_

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <unordered_map>

class Batch2D;
class Assets;
class Camera;

/*
 Some info about padding and margin.
    Padding is element inner space, margin is outer
    glm::vec4 usage:
      x - left
      y - top
      z - right
      w - bottom

 Outer element
 +======================================================================+
 |            .           .                    .          .             |
 |            .padding.y  .                    .          .             |
 | padding.x  .           .                    .          .   padding.z |
 |- - - - - - + - - - - - + - - - - - - - - - -+- - - - - + - - - - - - |
 |            .           .                    .          .             |
 |            .           .margin.y            .          .             |
 |            .margin.x   .                    .  margin.z.             |
 |- - - - - - + - - - - - +====================+- - - - - + - - - - - - |
 |            .           |    Inner element   |          .             |
 |- - - - - - + - - - - - +====================+- - - - - + - - - - - - |
 |            .           .                    .          .             |
 |            .           .margin.w            .          .             |
 |            .           .                    .          .             |
 |- - - - - - + - - - - - + - - - - - - - - - -+- - - - - + - - - - - - |
 |            .           .                    .          .             |
 |            .padding.w  .                    .          .             |
 |            .           .                    .          .             |
 +======================================================================+
*/

namespace gui {
    typedef std::function<void()> runnable;

    class UINode;
    class Container;
    class PagesControl;

    class GUI {
        Container* container;
        std::shared_ptr<UINode> hover = nullptr;
        std::shared_ptr<UINode> pressed = nullptr;
        std::shared_ptr<UINode> focus = nullptr;
        std::unordered_map<std::string, std::shared_ptr<UINode>> storage;

        Camera* uicamera;
        PagesControl* menu;
        void actMouse(float delta);
    public:
        GUI();
        ~GUI();

        PagesControl* getMenu();

        std::shared_ptr<UINode> getFocused() const;
        bool isFocusCaught() const;

        void act(float delta);
        void draw(Batch2D* batch, Assets* assets);
        void addBack(std::shared_ptr<UINode> panel);
        void add(std::shared_ptr<UINode> panel);
        void remove(std::shared_ptr<UINode> panel);
        void store(std::string name, std::shared_ptr<UINode> node);
        std::shared_ptr<UINode> get(std::string name);
        void remove(std::string name);
        void setFocus(std::shared_ptr<UINode> node);
    };
}

#endif // FRONTEND_GUI_GUI_H_