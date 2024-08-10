#pragma once

#include "commons.hpp"
#include "Container.hpp"

namespace gui {
    class Panel : public Container {
    protected:
        Orientation orientation = Orientation::vertical;
        glm::vec4 padding {2.0f};
        float interval = 2.0f;
        int maxLength = 0;
    public:
        Panel(
            glm::vec2 size, 
            glm::vec4 padding=glm::vec4(2.0f), 
            float interval=2.0f
        );
        virtual ~Panel();

        virtual void cropToContent();

        virtual void setOrientation(Orientation orientation);
        Orientation getOrientation() const;

        virtual void add(const std::shared_ptr<UINode> &node) override;

        virtual void refresh() override;
        virtual void fullRefresh() override;

        virtual void setMaxLength(int value);
        int getMaxLength() const;

        virtual void setPadding(glm::vec4 padding);
        glm::vec4 getPadding() const;
    };
}
