#include "GUI.hpp"

#include "gui_util.hpp"

#include "elements/UINode.hpp"
#include "elements/Label.hpp"
#include "elements/Menu.hpp"
#include "elements/Panel.hpp"

#include "assets/Assets.hpp"
#include "frontend/UiDocument.hpp"
#include "frontend/locale.hpp"
#include "graphics/core/Batch2D.hpp"
#include "graphics/core/LineBatch.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Font.hpp"
#include "graphics/core/DrawContext.hpp"
#include "window/Events.hpp"
#include "window/Window.hpp"
#include "window/input.hpp"
#include "window/Camera.hpp"

#include <algorithm>
#include <utility>

using namespace gui;

GUI::GUI()
    : batch2D(std::make_unique<Batch2D>(1024)),
      container(std::make_shared<Container>(glm::vec2(1000))) {
    container->setId("root");
    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);
    uicamera->perspective = false;
    uicamera->flipped = true;

    menu = std::make_shared<Menu>();
    menu->setId("menu");
    container->add(menu);
    container->setScrollable(false);

    tooltip = guiutil::create(
        "<container color='#000000A0' interactive='false' z-index='999'>"
            "<label id='tooltip.label' pos='2' autoresize='true'></label>"
        "</container>"
    );
    store("tooltip", tooltip);
    store("tooltip.label", UINode::find(tooltip, "tooltip.label"));
    container->add(tooltip);
}

GUI::~GUI() = default;

void GUI::setPageLoader(PageLoaderFunc pageLoader) {
    this->pagesLoader = std::move(pageLoader);
    menu->setPageLoader(this->pagesLoader);
}

PageLoaderFunc GUI::getPagesLoader() {
    return pagesLoader;
}

std::shared_ptr<Menu> GUI::getMenu() {
    return menu;
}

void GUI::onAssetsLoad(Assets* assets) {
    assets->store(std::make_unique<UiDocument>(
        "core:root", 
        uidocscript {}, 
        std::dynamic_pointer_cast<gui::UINode>(container), 
        nullptr
    ), "core:root");
}

void GUI::resetTooltip() {
    tooltipTimer = 0.0f;
    tooltip->setVisible(false);
}

void GUI::updateTooltip(float delta) {
    if (hover == nullptr || !hover->isInside(Events::cursor)) {
        return resetTooltip();
    }
    if (tooltipTimer + delta >= hover->getTooltipDelay()) {
        auto label = std::dynamic_pointer_cast<gui::Label>(get("tooltip.label"));
        const auto& text = hover->getTooltip();
        if (text.empty() && tooltip->isVisible()) {
            return resetTooltip();
        }
        if (label && !text.empty()) {
            tooltip->setVisible(true);
            label->setText(langs::get(text));
            auto size = label->getSize()+glm::vec2(4.0f);
            auto pos = Events::cursor+glm::vec2(10.0f);
            auto rootSize = container->getSize();
            pos.x = glm::min(pos.x, rootSize.x-size.x);
            pos.y = glm::min(pos.y, rootSize.y-size.y);
            tooltip->setSize(size);
            tooltip->setPos(pos);
        }
    }
    tooltipTimer += delta;
}

/// @brief Mouse related input and logic handling 
void GUI::actMouse(float delta) {
    float mouseDelta = glm::length(Events::delta);
    doubleClicked = false;
    doubleClickTimer += delta + mouseDelta * 0.1f;

    auto hover = container->getAt(Events::cursor);
    if (this->hover && this->hover != hover) {
        this->hover->setHover(false);
    }
    if (hover) {
        hover->setHover(true);
        if (Events::scroll) {
            hover->scrolled(Events::scroll);
        }
    }
    this->hover = hover;

    if (Events::jclicked(mousecode::BUTTON_1)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            if (doubleClickTimer < doubleClickDelay) {
                pressed->doubleClick(this, Events::cursor.x, Events::cursor.y);
                doubleClicked = true;
            } else {
                pressed->click(this, Events::cursor.x, Events::cursor.y);
            }
            doubleClickTimer = 0.0f;
            if (focus && focus != pressed) {
                focus->defocus();
            }
            if (focus != pressed) {
                focus = pressed;
                focus->onFocus(this);
                return;
            }
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (!Events::clicked(mousecode::BUTTON_1) && pressed) {
        pressed->mouseRelease(this, Events::cursor.x, Events::cursor.y);
        pressed = nullptr;
    }

    if (hover) {
        for (mousecode code : MOUSECODES_ALL) {
            if (Events::jclicked(code)) {
                hover->clicked(this, code);
            }
        }
    }
} 

void GUI::actFocused() {
    if (Events::jpressed(keycode::ESCAPE)) {
        focus->defocus();
        focus = nullptr;
        return;
    }
    for (auto codepoint : Events::codepoints) {
        focus->typed(codepoint);
    }
    for (auto key : Events::pressedKeys) {
        focus->keyPressed(key);
    }

    if (!Events::isCursorLocked()) {
        if (Events::clicked(mousecode::BUTTON_1) && 
            (Events::jclicked(mousecode::BUTTON_1) || Events::delta.x || Events::delta.y))
        {
            if (!doubleClicked) {
                focus->mouseMove(this, Events::cursor.x, Events::cursor.y);
            }
        }
    }
}

void GUI::act(float delta, const Viewport& vp) {
    container->setSize(vp.size());
    container->act(delta);
    auto prevfocus = focus;

    updateTooltip(delta);
    if (!Events::isCursorLocked()) {
        actMouse(delta);
    } else {
        if (hover) {
            hover->setHover(false);
            hover = nullptr;
        }
    }
    
    if (focus) {
        actFocused();
    }
    if (focus && !focus->isFocused()) {
        focus = nullptr;
    }
}

void GUI::postAct() {
    while (!postRunnables.empty()) {
        runnable callback = postRunnables.front();
        postRunnables.pop();
        callback();
    }
}

void GUI::draw(const DrawContext& pctx, const Assets& assets) {
    auto ctx = pctx.sub(batch2D.get());

    auto& viewport = ctx.getViewport();
    glm::vec2 wsize = viewport.size();

    auto& page = menu->getCurrent();
    if (page.panel) {
        menu->setSize(page.panel->getSize());
        page.panel->refresh();
        if (auto panel = std::dynamic_pointer_cast<gui::Panel>(page.panel)) {
            panel->cropToContent();
        }
    }
    menu->setPos((wsize - menu->getSize()) / 2.0f);
    uicamera->setFov(wsize.y);

    auto uishader = assets.get<Shader>("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    batch2D->begin();
    container->draw(ctx, assets);

    if (hover) {
        Window::setCursor(hover->getCursor());
    }
    if (hover && debug) {
        auto pos = hover->calcPos();
        const auto& id = hover->getId();
        if (!id.empty()) {
            auto& font = assets.require<Font>(FONT_DEFAULT);
            auto text = util::str2wstr_utf8(id);
            int width = font.calcWidth(text);
            int height = font.getLineHeight();

            batch2D->untexture();
            batch2D->setColor(0, 0, 0);
            batch2D->rect(pos.x, pos.y, width, height);

            batch2D->resetColor();
            font.draw(*batch2D, text, pos.x, pos.y, nullptr, 0);
        }

        batch2D->untexture();
        auto node = hover->getParent();
        while (node) {
            auto pos = node->calcPos();
            auto size = node->getSize();

            batch2D->setColor(0, 255, 255);
            batch2D->lineRect(pos.x, pos.y, size.x-1, size.y-1);

            node = node->getParent();
        }
        // debug draw
        auto size = hover->getSize();
        batch2D->setColor(0, 255, 0);
        batch2D->lineRect(pos.x, pos.y, size.x-1, size.y-1);
    }
}

std::shared_ptr<UINode> GUI::getFocused() const {
    return focus;
}

bool GUI::isFocusCaught() const {
    return focus && focus->isFocuskeeper();
}

void GUI::add(std::shared_ptr<UINode> node) {
    container->add(std::move(node));
}

void GUI::remove(UINode* node) noexcept {
    container->remove(node);
}

void GUI::store(const std::string& name, std::shared_ptr<UINode> node) {
    storage[name] = std::move(node);
}

std::shared_ptr<UINode> GUI::get(const std::string& name) noexcept {
    auto found = storage.find(name);
    if (found == storage.end()) {
        return nullptr;
    }
    return found->second;
}

void GUI::remove(const std::string& name) noexcept {
    storage.erase(name);
}

void GUI::setFocus(std::shared_ptr<UINode> node) {
    if (focus) {
        focus->defocus();
    }
    focus = std::move(node);
    if (focus) {
        focus->onFocus(this);
    }
}

std::shared_ptr<Container> GUI::getContainer() const {
    return container;
}

void GUI::postRunnable(const runnable& callback) {
    postRunnables.push(callback);
}

void GUI::setDoubleClickDelay(float delay) {
    doubleClickDelay = delay;
}

float GUI::getDoubleClickDelay() const {
    return doubleClickDelay;
}

void GUI::toggleDebug() {
    debug = !debug;
}
