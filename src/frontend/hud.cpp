#include "hud.hpp"

#include "ContentGfxCache.hpp"
#include "LevelFrontend.hpp"
#include "UiDocument.hpp"

#include "../assets/Assets.hpp"
#include "../content/Content.hpp"
#include "../core_defs.hpp"
#include "../delegates.hpp"
#include "../engine.hpp"
#include "../graphics/core/Atlas.hpp"
#include "../graphics/core/Batch2D.hpp"
#include "../graphics/core/Batch3D.hpp"
#include "../graphics/core/DrawContext.hpp"
#include "../graphics/core/Font.hpp"
#include "../graphics/core/Mesh.hpp"
#include "../graphics/core/Shader.hpp"
#include "../graphics/core/Texture.hpp"
#include "../graphics/render/WorldRenderer.hpp"
#include "../graphics/ui/elements/InventoryView.hpp"
#include "../graphics/ui/elements/Menu.hpp"
#include "../graphics/ui/elements/Panel.hpp"
#include "../graphics/ui/elements/Plotter.hpp"
#include "../graphics/ui/elements/UINode.hpp"
#include "../graphics/ui/gui_util.hpp"
#include "../graphics/ui/GUI.hpp"
#include "../items/Inventories.hpp"
#include "../items/Inventory.hpp"
#include "../items/ItemDef.hpp"
#include "../logic/scripting/scripting.hpp"
#include "../maths/voxmaths.hpp"
#include "../objects/Player.hpp"
#include "../physics/Hitbox.hpp"
#include "../typedefs.hpp"
#include "../util/stringutil.hpp"
#include "../voxels/Block.hpp"
#include "../voxels/Chunk.hpp"
#include "../voxels/Chunks.hpp"
#include "../window/Camera.hpp"
#include "../window/Events.hpp"
#include "../window/input.hpp"
#include "../window/Window.hpp"
#include "../world/Level.hpp"
#include "../world/World.hpp"

#include <assert.h>
#include <memory>
#include <stdexcept>
#include <string>

using namespace gui;

// implemented in debug_panel.cpp
extern std::shared_ptr<UINode> create_debug_panel(
    Engine* engine, 
    Level* level, 
    Player* player
);

HudElement::HudElement(
    hud_element_mode mode, 
    UiDocument* document, 
    std::shared_ptr<UINode> node, 
    bool debug
) : mode(mode), document(document), node(node), debug(debug) {
}

void HudElement::update(bool pause, bool inventoryOpen, bool debugMode) {
    if (debug && !debugMode) {
        node->setVisible(false);
        return;
    }
    switch (mode) {
        case hud_element_mode::permanent:
            node->setVisible(true);
            break;
        case hud_element_mode::ingame:
            node->setVisible(!pause && !inventoryOpen);
            break;
        case hud_element_mode::inventory_any:
            node->setVisible(inventoryOpen);
            break;
        case hud_element_mode::inventory_bound:
            removed = !inventoryOpen;
            break;
    }
}

UiDocument* HudElement::getDocument() const {
    return document;
}

std::shared_ptr<UINode> HudElement::getNode() const {
    return node;
}

std::shared_ptr<InventoryView> Hud::createContentAccess() {
    auto level = frontend->getLevel();
    auto content = level->content;
    auto indices = content->getIndices();
    auto inventory = player->getInventory();
    
    int itemsCount = indices->countItemDefs();
    auto accessInventory = std::make_shared<Inventory>(0, itemsCount);
    for (int id = 1; id < itemsCount; id++) {
        accessInventory->getSlot(id-1).set(ItemStack(id, 1));
    }

    SlotLayout slotLayout(-1, glm::vec2(), false, true, nullptr,
    [=](uint, ItemStack& item) {
        auto copy = ItemStack(item);
        inventory->move(copy, indices);
    }, 
    [=](uint, ItemStack& item) {
        inventory->getSlot(player->getChosenSlot()).set(item);
    });

    InventoryBuilder builder;
    builder.addGrid(8, itemsCount-1, glm::vec2(), 8, true, slotLayout);
    auto view = builder.build();
    view->bind(accessInventory, content);
    view->setMargin(glm::vec4());
    return view;
}

std::shared_ptr<InventoryView> Hud::createHotbar() {
    auto inventory = player->getInventory();
    auto content = frontend->getLevel()->content;

    SlotLayout slotLayout(-1, glm::vec2(), false, false, nullptr, nullptr, nullptr);
    InventoryBuilder builder;
    builder.addGrid(10, 10, glm::vec2(), 4, true, slotLayout);
    auto view = builder.build();

    view->setOrigin(glm::vec2(view->getSize().x/2, 0));
    view->bind(inventory, content);
    view->setInteractive(false);
    return view;
}

Hud::Hud(Engine* engine, LevelFrontend* frontend, Player* player) 
  : assets(engine->getAssets()), 
    gui(engine->getGUI()),
    frontend(frontend),
    player(player)
{
    contentAccess = createContentAccess();
    contentAccessPanel = std::make_shared<Panel>(
        contentAccess->getSize(), glm::vec4(0.0f), 0.0f
    );
    contentAccessPanel->setColor(glm::vec4());
    contentAccessPanel->add(contentAccess);
    contentAccessPanel->setScrollable(true);

    hotbarView = createHotbar();
    darkOverlay = guiutil::create(
        "<container size='4000' color='#00000080' z-index='-1' visible='false'/>"
    );

    uicamera = std::make_unique<Camera>(glm::vec3(), 1);
    uicamera->perspective = false;
    uicamera->flipped = true;

    debugPanel = create_debug_panel(engine, frontend->getLevel(), player);
    debugPanel->setZIndex(2);
    
    gui->add(darkOverlay);
    gui->add(hotbarView);
    gui->add(debugPanel);
    gui->add(contentAccessPanel);

    auto dplotter = std::make_shared<Plotter>(350, 250, 2000, 16);
    dplotter->setGravity(Gravity::bottom_right);
    add(HudElement(hud_element_mode::permanent, nullptr, dplotter, true));
}

Hud::~Hud() {
    // removing all controlled ui
    for (auto& element : elements) {
        onRemove(element);
    }
    gui->remove(hotbarView);
    gui->remove(darkOverlay);
    gui->remove(contentAccessPanel);
    gui->remove(debugPanel);
}

/// @brief Remove all elements marked as removed
void Hud::cleanup() {
    auto it = std::remove_if(elements.begin(), elements.end(), [](const HudElement& e) {
        return e.isRemoved();
    });
    elements.erase(it, elements.end());
}

void Hud::processInput(bool visible) {
    if (Events::jpressed(keycode::ESCAPE)) {
        if (pause) {
            setPause(false);
        } else if (inventoryOpen) {
            closeInventory();
        } else {
            setPause(true);
        }
    }
    if (!pause && Events::active(BIND_DEVTOOLS_CONSOLE)) {
        showOverlay(assets->getLayout("core:console"), false);
    }
    if (!Window::isFocused() && !pause && !isInventoryOpen()) {
        setPause(true);
    }

    if (!pause && visible && Events::jactive(BIND_HUD_INVENTORY)) {
        if (inventoryOpen) {
            closeInventory();
        } else {
            openInventory();
        }
    }
    if (!pause) {
        updateHotbarControl();
    }
}

void Hud::updateHotbarControl() {
    if (!inventoryOpen && Events::scroll) {
        int slot = player->getChosenSlot();
        slot = (slot - Events::scroll) % 10;
        if (slot < 0) {
            slot += 10;
        }
        player->setChosenSlot(slot);
    }
    for (
        int i = static_cast<int>(keycode::NUM_1); 
        i <= static_cast<int>(keycode::NUM_9); 
        i++
    ) {
        if (Events::jpressed(i)) {
            player->setChosenSlot(i - static_cast<int>(keycode::NUM_1));
        }
    }
    if (Events::jpressed(keycode::NUM_0)) {
        player->setChosenSlot(9);
    }
}

void Hud::update(bool visible) {
    auto level = frontend->getLevel();
    auto menu = gui->getMenu();

    debugPanel->setVisible(player->debug && visible);

    if (!visible && inventoryOpen) {
        closeInventory();
    }
    if (pause && menu->getCurrent().panel == nullptr) {
        setPause(false);
    }

    if (!gui->isFocusCaught()) {
        processInput(visible);
    }
    if ((pause || inventoryOpen) == Events::_cursor_locked) {
        Events::toggleCursor();
    }

    if (blockUI) {
        voxel* vox = level->chunks->get(blockPos.x, blockPos.y, blockPos.z);
        if (vox == nullptr || vox->id != currentblockid) {
            closeInventory();
        }
    }

    for (auto& element : elements) {
        element.getNode()->setVisible(visible);
    }

    glm::vec2 invSize = contentAccessPanel->getSize();
    contentAccessPanel->setVisible(inventoryView != nullptr);
    contentAccessPanel->setSize(glm::vec2(invSize.x, Window::height));
    contentAccess->setMinSize(glm::vec2(1, Window::height));
    hotbarView->setVisible(visible);

    if (visible) {
        for (auto& element : elements) {
            element.update(pause, inventoryOpen, player->debug);
            if (element.isRemoved()) {
                onRemove(element);
            }
        }
    }
    cleanup();
}

/// @brief Show inventory on the screen and turn on inventory mode blocking movement
void Hud::openInventory() {
    auto level = frontend->getLevel();
    auto content = level->content;
    showExchangeSlot();

    inventoryOpen = true;
    auto inventory = player->getInventory();
    auto inventoryDocument = assets->getLayout("core:inventory");
    inventoryView = std::dynamic_pointer_cast<InventoryView>(inventoryDocument->getRoot());
    inventoryView->bind(inventory, content);
    add(HudElement(hud_element_mode::inventory_bound, inventoryDocument, inventoryView, false));
    add(HudElement(hud_element_mode::inventory_bound, nullptr, exchangeSlot, false));
}

void Hud::openInventory(
    glm::ivec3 block, 
    UiDocument* doc, 
    std::shared_ptr<Inventory> blockinv, 
    bool playerInventory
) {
    if (isInventoryOpen()) {
        closeInventory();
    }
    auto level = frontend->getLevel();
    auto content = level->content;
    blockUI = std::dynamic_pointer_cast<InventoryView>(doc->getRoot());
    if (blockUI == nullptr) {
        throw std::runtime_error("block UI root element must be 'inventory'");
    }
    secondUI = blockUI;
    if (playerInventory) {
        openInventory();
    } else {
        inventoryOpen = true;
    }
    if (blockinv == nullptr) {
        blockinv = level->inventories->createVirtual(blockUI->getSlotsCount());
    }
    level->chunks->getChunkByVoxel(block.x, block.y, block.z)->setUnsaved(true);
    blockUI->bind(blockinv, content);
    blockPos = block;
    currentblockid = level->chunks->get(block.x, block.y, block.z)->id;
    add(HudElement(hud_element_mode::inventory_bound, doc, blockUI, false));
}

void Hud::showExchangeSlot() {
    auto level = frontend->getLevel();
    auto content = level->content;
    exchangeSlotInv = level->inventories->createVirtual(1);
    exchangeSlot = std::make_shared<SlotView>(
        SlotLayout(-1, glm::vec2(), false, false, nullptr, nullptr, nullptr)
    );
    exchangeSlot->bind(exchangeSlotInv->getId(), exchangeSlotInv->getSlot(0), content);
    exchangeSlot->setColor(glm::vec4());
    exchangeSlot->setInteractive(false);
    exchangeSlot->setZIndex(1);
    gui->store(SlotView::EXCHANGE_SLOT_NAME, exchangeSlot);

}

void Hud::showOverlay(UiDocument* doc, bool playerInventory) {
    if (isInventoryOpen()) {
        closeInventory();
    }
    secondUI = doc->getRoot();
    if (playerInventory) {
        openInventory();
    } else {
        showExchangeSlot();
        inventoryOpen = true;
    }
    add(HudElement(hud_element_mode::inventory_bound, doc, secondUI, false));
}

void Hud::openPermanent(UiDocument* doc) {
    auto root = doc->getRoot();
    remove(root);

    auto invview = std::dynamic_pointer_cast<InventoryView>(root);
    if (invview) {
        invview->bind(player->getInventory(), frontend->getLevel()->content);
    }
    add(HudElement(hud_element_mode::permanent, doc, doc->getRoot(), false));
}

void Hud::closeInventory() {
    gui->remove(SlotView::EXCHANGE_SLOT_NAME);
    exchangeSlot = nullptr;
    exchangeSlotInv = nullptr;
    inventoryOpen = false;
    inventoryView = nullptr;
    blockUI = nullptr;
    secondUI = nullptr;

    for (auto& element : elements) {
        if (element.isInventoryBound()) {
            element.setRemoved();
            onRemove(element);
        }
    }
    cleanup();
}

void Hud::add(HudElement element) {
    using namespace dynamic;

    gui->add(element.getNode());
    auto invview = std::dynamic_pointer_cast<InventoryView>(element.getNode());
    auto document = element.getDocument();
    if (document) {
        auto inventory = invview ? invview->getInventory() : nullptr;
        std::vector<Value> args;
        args.push_back(inventory ? inventory.get()->getId() : 0);
        for (int i = 0; i < 3; i++) {
            args.push_back(static_cast<integer_t>(blockPos[i]));
        }
        scripting::on_ui_open(
            element.getDocument(), 
            std::move(args)
        );
    }
    elements.push_back(element);
}

void Hud::onRemove(const HudElement& element) {
    auto document = element.getDocument();
    if (document) {
        Inventory* inventory = nullptr;
        auto invview = std::dynamic_pointer_cast<InventoryView>(element.getNode());
        if (invview) {
            inventory = invview->getInventory().get();
        }
        scripting::on_ui_close(document, inventory);
        if (invview) {
            invview->unbind();
        }
    }
    gui->remove(element.getNode());
}

void Hud::remove(std::shared_ptr<UINode> node) {
    for (auto& element : elements) {
        if (element.getNode() == node) {
            element.setRemoved();
            onRemove(element);
        }
    }
    cleanup();
}

void Hud::draw(const DrawContext& ctx){
    const Viewport& viewport = ctx.getViewport();
    const uint width = viewport.getWidth();
    const uint height = viewport.getHeight();

    updateElementsPosition(viewport);

    uicamera->setFov(height);

    auto batch = ctx.getBatch2D();
    batch->begin();

    Shader* uishader = assets->getShader("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    // Crosshair
    if (!pause && !inventoryOpen && !player->debug) {
        DrawContext chctx = ctx.sub();
        chctx.setBlendMode(BlendMode::inversion);
        auto texture = assets->getTexture("gui/crosshair");
        batch->texture(texture);
        int chsizex = texture != nullptr ? texture->getWidth() : 16;
        int chsizey = texture != nullptr ? texture->getHeight() : 16;
        batch->rect(
            (width-chsizex)/2, (height-chsizey)/2, 
            chsizex, chsizey, 0,0, 1,1, 1,1,1,1
        );
    }
}

void Hud::updateElementsPosition(const Viewport& viewport) {
    const uint width = viewport.getWidth();
    const uint height = viewport.getHeight();
    
    if (inventoryOpen) {
        float caWidth = inventoryView ? contentAccess->getSize().x : 0.0f;
        contentAccessPanel->setPos(glm::vec2(width-caWidth, 0));

        glm::vec2 invSize = inventoryView ? inventoryView->getSize() : glm::vec2();
        if (secondUI == nullptr) {
            if (inventoryView) {
                inventoryView->setPos(glm::vec2(
                    glm::min(width/2-invSize.x/2, width-caWidth-10-invSize.x),
                    height/2-invSize.y/2
                ));
            }
        } else {
            glm::vec2 secondUISize = secondUI->getSize();
            float invwidth = glm::max(invSize.x, secondUISize.x);
            int interval = invSize.y > 0.0 ? 5 : 0;
            float totalHeight = invSize.y + secondUISize.y + interval;
            if (inventoryView) {
                inventoryView->setPos(glm::vec2(
                    glm::min(width/2-invwidth/2, width-caWidth-10-invwidth),
                    height/2+totalHeight/2-invSize.y
                ));
            }
            secondUI->setPos(glm::vec2(
                glm::min(width/2-invwidth/2, width-caWidth-(inventoryView ? 10 : 0)-invwidth),
                height/2-totalHeight/2
            ));
        }
    }
    if (exchangeSlot != nullptr) {
        exchangeSlot->setPos(glm::vec2(Events::cursor));
    }
    hotbarView->setPos(glm::vec2(width/2, height-65));
    hotbarView->setSelected(player->getChosenSlot());
}

bool Hud::isInventoryOpen() const {
    return inventoryOpen;
}

bool Hud::isPause() const {
    return pause;
}

void Hud::setPause(bool pause) {
    if (this->pause == pause) {
        return;
    }
    this->pause = pause;

    if (inventoryOpen) {
        closeInventory();
    }
    
    auto menu = gui->getMenu();
    if (pause) {
        menu->setPage("pause");
    } else {
        menu->reset();
    }
    darkOverlay->setVisible(pause);
    menu->setVisible(pause);
}

Player* Hud::getPlayer() const {
    return player;
}

std::shared_ptr<Inventory> Hud::getBlockInventory() {
    if (blockUI == nullptr) {
        return nullptr;
    }
    return blockUI->getInventory();
}
