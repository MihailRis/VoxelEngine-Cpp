#pragma once

#include "commons.hpp"
#include "BasePanel.hpp"

namespace gui {
    class Panel : public BasePanel {
    public:
        Panel(
            glm::vec2 size, 
            glm::vec4 padding=glm::vec4(0.0f), 
            float interval=2.0f
        );
        virtual ~Panel();

        virtual void cropToContent();

        virtual void add(const std::shared_ptr<UINode>& node) override;
        virtual void remove(UINode* node) override;

        virtual void refresh() override;
        virtual void fullRefresh() override;

        virtual void setMaxLength(int value);
        int getMaxLength() const;

        virtual void setMinLength(int value);
        int getMinLength() const;
    protected:
        int minLength = 0;
        int maxLength = 0;
    };
}
