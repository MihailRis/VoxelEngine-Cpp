#ifndef FRONTEND_GUI_PANEL_H_
#define FRONTEND_GUI_PANEL_H_

#include <glm/glm.hpp>

class Batch2D;

namespace gui {
    class Panel {
        glm::vec2 coord;
        glm::vec2 size;
        glm::vec4 color;
        bool visible = true;
    public:
        Panel(glm::vec2 coord, glm::vec2 size, glm::vec4 color);

        void draw(Batch2D* batch);

        void setVisible(bool flag);
        bool isVisible() const;
    };
}

#endif // FRONTEND_GUI_PANEL_H_