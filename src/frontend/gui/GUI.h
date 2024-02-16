#ifndef FRONTEND_GUI_GUI_H_
#define FRONTEND_GUI_GUI_H_

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <unordered_map>

class GfxContext;
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
    class UINode;
    class Container;
    class PagesControl;

    /** The main UI controller */
    class GUI {
        std::shared_ptr<Container> container;
        std::shared_ptr<UINode> hover = nullptr;
        std::shared_ptr<UINode> pressed = nullptr;
        std::shared_ptr<UINode> focus = nullptr;
        std::unordered_map<std::string, std::shared_ptr<UINode>> storage;

        std::unique_ptr<Camera> uicamera;
        std::shared_ptr<PagesControl> menu;
        void actMouse(float delta);
    public:
        GUI();
        ~GUI();

        std::shared_ptr<PagesControl> getMenu();

        std::shared_ptr<UINode> getFocused() const;
        bool isFocusCaught() const;

        void act(float delta);
        void draw(const GfxContext* pctx, Assets* assets);
        void addBack(std::shared_ptr<UINode> panel);
        void add(std::shared_ptr<UINode> panel);
        void remove(std::shared_ptr<UINode> panel);
        void store(std::string name, std::shared_ptr<UINode> node);
        std::shared_ptr<UINode> get(std::string name);
        void remove(std::string name);
        void setFocus(std::shared_ptr<UINode> node);

        std::shared_ptr<Container> getContainer() const;
    };
}

#endif // FRONTEND_GUI_GUI_H_
