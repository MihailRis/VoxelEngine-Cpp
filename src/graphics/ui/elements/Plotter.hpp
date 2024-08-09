#ifndef GRAPHICS_UI_ELEMENTS_PLOTTER_HPP_
#define GRAPHICS_UI_ELEMENTS_PLOTTER_HPP_

#include "UINode.hpp"
#include "typedefs.hpp"

#include <memory>
#include <glm/glm.hpp>

class Assets;
class DrawContext;

namespace gui {
    class Plotter : public gui::UINode {
        std::unique_ptr<int[]> points;
        float multiplier;
        int index = 0;
        int dmwidth;
        int dmheight;
        int labelsInterval;
    public:
        Plotter(uint width, uint height, float multiplier, int labelsInterval) 
        : gui::UINode(glm::vec2(width, height)), 
            multiplier(multiplier),
            dmwidth(width-50),
            dmheight(height),
            labelsInterval(labelsInterval)
        {
            points = std::make_unique<int[]>(dmwidth);
        }

        void act(float delta) override;
        void draw(const DrawContext* pctx, Assets* assets) override;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_PLOTTER_HPP_
