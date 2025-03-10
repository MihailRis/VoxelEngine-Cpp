#pragma once

#include "BasePanel.hpp"

namespace gui {
    class SplitBox : public BasePanel {
    public:
        SplitBox(const glm::vec2& size, float splitPos, Orientation orientation);
        
        virtual void mouseMove(GUI*, int x, int y) override;
        virtual void refresh() override;
        virtual void fullRefresh() override;
    private:
        float splitPos;
    };
}
