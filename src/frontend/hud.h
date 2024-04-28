#ifndef FRONTEND_HUD_H_
#define FRONTEND_HUD_H_

#include "../typedefs.h"

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

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
class GfxContext;
class Viewport;

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
    Assets* assets;
    std::unique_ptr<Camera> uicamera;
    gui::GUI* gui;
    LevelFrontend* frontend;
    Player* player;

    /// @brief Is any overlay/inventory open
    bool inventoryOpen = false;
    /// @brief Is pause mode on
    bool pause = false;

    /// @brief Content access panel scroll container
    std::shared_ptr<gui::Container> contentAccessPanel;
    /// @brief Content access panel itself
    std::shared_ptr<InventoryView> contentAccess;
    /// @brief Player inventory hotbar
    std::shared_ptr<InventoryView> hotbarView;
    /// @brief Debug info and control panel (F3 key)
    std::shared_ptr<gui::UINode> debugPanel;
    /// @brief Overlay used in pause mode
    std::shared_ptr<gui::UINode> darkOverlay;
    /// @brief Inventories interaction agent (grabbed item)
    std::shared_ptr<SlotView> exchangeSlot;
    /// @brief Exchange slot inventory (1 slot only)
    std::shared_ptr<Inventory> exchangeSlotInv = nullptr;
    /// @brief List of all controlled hud elements
    std::vector<HudElement> elements;

    /// @brief Player inventory view
    std::shared_ptr<InventoryView> inventoryView = nullptr;
    /// @brief Block inventory view
    std::shared_ptr<InventoryView> blockUI = nullptr;
    /// @brief Position of the block open
    glm::ivec3 blockPos {};
    /// @brief Id of the block open (used to detect block destruction or replacement)
    blockid_t currentblockid = 0;

    /// @brief UI element will be dynamicly positioned near to inventory or in screen center
    std::shared_ptr<gui::UINode> secondUI = nullptr;
    
    std::shared_ptr<InventoryView> createContentAccess();
    std::shared_ptr<InventoryView> createHotbar();

    void processInput(bool visible);
    void updateElementsPosition(const Viewport& viewport);
    void updateHotbarControl();
    void cleanup();
public:
    Hud(Engine* engine, LevelFrontend* frontend, Player* player);
    ~Hud();

    void update(bool hudVisible);
    void draw(const GfxContext& context);

    /// @brief Check if inventory mode on
    bool isInventoryOpen() const;

    /// @brief Check if pause mode on
    bool isPause() const;

    /// @brief Enable/disable pause mode
    void setPause(bool pause);

    /// @brief Show player inventory in inventory-mode
    void openInventory();
    
    /// @brief Show block inventory in inventory-mode
    /// @param block block position
    /// @param doc block ui layout
    /// @param blockInv block inventory
    /// @param playerInventory show player inventory too
    void openInventory(
        glm::ivec3 block, 
        UiDocument* doc, 
        std::shared_ptr<Inventory> blockInv, 
        bool playerInventory
    );

    /// @brief Show element in inventory-mode
    /// @param doc element layout
    /// @param playerInventory show player inventory too
    void showOverlay(UiDocument* doc, bool playerInventory);

    /// @brief Close all open inventories and overlay
    void closeInventory();

    /// @brief Add element will be visible until removed
    /// @param doc element layout
    void openPermanent(UiDocument* doc);

    void add(HudElement element);
    void onRemove(HudElement& element);
    void remove(std::shared_ptr<gui::UINode> node);

    Player* getPlayer() const;

    std::shared_ptr<Inventory> getBlockInventory();
};

#endif // FRONTEND_HUD_H_
