#ifndef FRONTEND_GUI_GUI_H_
#define FRONTEND_GUI_GUI_H_

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <functional>

class Batch2D;
class Assets;

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

    typedef std::function<void()> ontimeout;
    struct IntervalEvent {
        ontimeout callback;
        float interval;
        float timer;
    };

    class GUI {
        Container* container;
        std::shared_ptr<UINode> hover = nullptr;
        std::shared_ptr<UINode> pressed = nullptr;
        std::shared_ptr<UINode> focus = nullptr;
        std::vector<IntervalEvent> intervalEvents;
    public:
        GUI();
        ~GUI();

        std::shared_ptr<UINode> getFocused() const;
        bool isFocusCaught() const;

        void act(float delta);
        void draw(Batch2D* batch, Assets* assets);
        void add(std::shared_ptr<UINode> panel);

        void interval(float interval, ontimeout callback);
    };
}

#endif // FRONTEND_GUI_GUI_H_