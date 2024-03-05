#ifndef SRC_HUD_H_
#define SRC_HUD_H_

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "../graphics/GfxContext.h"

class Camera;
class Block;
class Assets;
class Player;
class Engine;
class SlotView;
class Inventory;
class InventoryView;
class LevelFrontend;
class UiDocument;
class InventoryInteraction;

namespace gui {
    class GUI;
    class UINode;
    class Panel;
    class Container;
}

enum class hud_element_mode {
    // element is hidden if menu or inventory open
    ingame,
    // element is visible if hud is visible
    permanent,
    // element is visible in inventory mode
    inventory_any,
    // element will be removed on inventory close
    inventory_bound
};

class HudElement {
    hud_element_mode mode;
    UiDocument* document;
    std::shared_ptr<gui::UINode> node;

    bool debug;
    bool removed = false;
public:
    HudElement(hud_element_mode mode, UiDocument* document, std::shared_ptr<gui::UINode> node, bool debug);

    void update(bool pause, bool inventoryOpen, bool debug);

    UiDocument* getDocument() const;
    std::shared_ptr<gui::UINode> getNode() const;

    void setRemoved() {
        removed = true;
    }

    bool isRemoved() const {
        return removed;
    }
};

class Hud {
    Engine* engine;
    Assets* assets;
    std::unique_ptr<Camera> uicamera;
    gui::GUI* gui;
    LevelFrontend* frontend;
    Player* player;

    bool inventoryOpen = false;
    bool pause = false;

    std::shared_ptr<gui::Container> contentAccessPanel;
    std::shared_ptr<InventoryView> contentAccess;
    std::shared_ptr<InventoryView> hotbarView;
    std::shared_ptr<gui::UINode> debugPanel;
    std::shared_ptr<gui::Panel> darkOverlay;
    std::unique_ptr<InventoryInteraction> interaction;
    std::shared_ptr<SlotView> grabbedItemView;
    std::vector<HudElement> elements;

    std::shared_ptr<InventoryView> inventoryView = nullptr;
    std::shared_ptr<InventoryView> blockUI = nullptr;
    glm::ivec3 currentblock {};
    blockid_t currentblockid = 0;
    
    std::shared_ptr<InventoryView> createContentAccess();
    std::shared_ptr<InventoryView> createHotbar();

    void cleanup();
public:
    Hud(Engine* engine, LevelFrontend* frontend, Player* player);
    ~Hud();

    void update(bool hudVisible);
    void draw(const GfxContext& context);

    bool isInventoryOpen() const;
    bool isPause() const;
    void setPause(bool pause);

    void openInventory();
    void openInventory(glm::ivec3 block, UiDocument* doc, std::shared_ptr<Inventory> blockInv, bool playerInventory);
    void closeInventory();
    void openPermanent(UiDocument* doc);

    void add(HudElement element);
    void remove(HudElement& element);
    void remove(std::shared_ptr<gui::UINode> node);

    Player* getPlayer() const;
};

#endif /* SRC_HUD_H_ */
