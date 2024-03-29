#include "hud.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <assert.h>
#include <stdexcept>

#include "../typedefs.h"
#include "../content/Content.h"
#include "../util/stringutil.h"
#include "../util/timeutil.h"
#include "../assets/Assets.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/Batch3D.h"
#include "../graphics/Font.h"
#include "../graphics/Atlas.h"
#include "../graphics/Mesh.h"
#include "../graphics/Texture.h"
#include "../window/Camera.h"
#include "../window/Window.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../voxels/ChunksMatrix.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../maths/voxmaths.h"
#include "gui/controls.h"
#include "gui/containers.h"
#include "gui/UINode.h"
#include "gui/GUI.h"
#include "ContentGfxCache.h"
#include "menu/menu.h"
#include "screens.h"
#include "WorldRenderer.h"
#include "BlocksPreview.h"
#include "InventoryView.h"
#include "LevelFrontend.h"
#include "UiDocument.h"
#include "../engine.h"
#include "../delegates.h"
#include "../core_defs.h"
#include "../items/ItemDef.h"
#include "../items/Inventory.h"
#include "../items/Inventories.h"
#include "../logic/scripting/scripting.h"


// implemented in debug_panel.cpp
extern std::shared_ptr<gui::UINode> create_debug_panel(
    Engine* engine, 
    Level* level, 
    Player* player
);

class DeltaGrapher : public gui::UINode {
    std::unique_ptr<int[]> points;
    float multiplier;
    int index = 0;
    int dmwidth;
    int dmheight;
public:
    DeltaGrapher(uint width, uint height, float multiplier) 
      : gui::UINode(glm::vec2(width, height)), 
        multiplier(multiplier),
        dmwidth(width),
        dmheight(height)
    {
        points = std::make_unique<int[]>(width);
    }

    void act(float delta) override {
        index = index + 1 % dmwidth;
        int value = static_cast<int>(delta * multiplier);
        points[index % dmwidth] = std::min(value, dmheight);
    }

    void draw(const GfxContext* pctx, Assets* assets) override {
        glm::vec2 pos = calcPos();
        auto batch = pctx->getBatch2D();
        batch->texture(nullptr);
        batch->lineWidth(1);
        for (int i = index+1; i < index+dmwidth; i++) {
            int j = i % dmwidth;
            batch->line(
                pos.x + i - index, pos.y + size.y - points[j], 
                pos.x + i - index, pos.y + size.y, 1.0f, 1.0f, 1.0f, 0.2f
            );
        }
    }
};

HudElement::HudElement(
    hud_element_mode mode, 
    UiDocument* document, 
    std::shared_ptr<gui::UINode> node, 
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

std::shared_ptr<gui::UINode> HudElement::getNode() const {
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
    view->bind(accessInventory, frontend, interaction.get());
    view->setMargin(glm::vec4());
    return view;
}

std::shared_ptr<InventoryView> Hud::createHotbar() {
    auto inventory = player->getInventory();

    SlotLayout slotLayout(-1, glm::vec2(), false, false, nullptr, nullptr, nullptr);
    InventoryBuilder builder;
    builder.addGrid(10, 10, glm::vec2(), 4, true, slotLayout);
    auto view = builder.build();

    view->setOrigin(glm::vec2(view->getSize().x/2, 0));
    view->bind(inventory, frontend, interaction.get());
    view->setInteractive(false);
    return view;
}

Hud::Hud(Engine* engine, LevelFrontend* frontend, Player* player) 
  : engine(engine),
    assets(engine->getAssets()), 
    gui(engine->getGUI()),
    frontend(frontend),
    player(player)
{
    interaction = std::make_unique<InventoryInteraction>();
    grabbedItemView = std::make_shared<SlotView>(
        SlotLayout(-1, glm::vec2(), false, false, nullptr, nullptr, nullptr)
    );
    grabbedItemView->bind(
        0,
        interaction->getGrabbedItem(), 
        frontend, 
        interaction.get()
    );
    grabbedItemView->setColor(glm::vec4());
    grabbedItemView->setInteractive(false);
    grabbedItemView->setZIndex(1);

    contentAccess = createContentAccess();
    contentAccessPanel = std::make_shared<gui::Panel>(
        contentAccess->getSize(), glm::vec4(0.0f), 0.0f
    );
    contentAccessPanel->setColor(glm::vec4());
    contentAccessPanel->add(contentAccess);
    contentAccessPanel->setScrollable(true);

    hotbarView = createHotbar();
    darkOverlay = std::make_unique<gui::Panel>(glm::vec2(4000.0f));
    darkOverlay->setColor(glm::vec4(0, 0, 0, 0.5f));
    darkOverlay->setZIndex(-1);
    darkOverlay->setVisible(false);

    uicamera = std::make_unique<Camera>(glm::vec3(), 1);
    uicamera->perspective = false;
    uicamera->flipped = true;

    debugPanel = create_debug_panel(engine, frontend->getLevel(), player);
    debugPanel->setZIndex(2);
    
    gui->add(darkOverlay);
    gui->add(hotbarView);
    gui->add(debugPanel);
    gui->add(contentAccessPanel);
    gui->add(grabbedItemView);

    auto dgrapher = std::make_shared<DeltaGrapher>(350, 250, 2000);
    dgrapher->setGravity(gui::Gravity::bottom_right);
    add(HudElement(hud_element_mode::permanent, nullptr, dgrapher, true));
}

Hud::~Hud() {
    // removing all controlled ui
    gui->remove(grabbedItemView);
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

    if (!pause && visible && Events::jactive(BIND_HUD_INVENTORY)) {
        if (inventoryOpen) {
            closeInventory();
        } else {
            openInventory();
        }
    }
    if (!pause) {
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
}

void Hud::update(bool visible) {
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
        voxel* vox = player->chunksMatrix->getVoxel(blockPos.x, blockPos.y, blockPos.z);
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
    inventoryOpen = true;
    auto inventory = player->getInventory();
    auto inventoryDocument = assets->getLayout("core:inventory");
    inventoryView = std::dynamic_pointer_cast<InventoryView>(inventoryDocument->getRoot());
    inventoryView->bind(inventory, frontend, interaction.get());
    add(HudElement(hud_element_mode::inventory_bound, inventoryDocument, inventoryView, false));
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
    player->chunksMatrix->getChunkByVoxel(block.x, block.y, block.z)->setUnsaved(true);
    blockUI->bind(blockinv, frontend, interaction.get());
    blockPos = block;
    currentblockid = player->chunksMatrix->getVoxel(block.x, block.y, block.z)->id;
    add(HudElement(hud_element_mode::inventory_bound, doc, blockUI, false));
}

void Hud::showOverlay(UiDocument* doc, bool playerInventory) {
    if (isInventoryOpen()) {
        closeInventory();
    }
    secondUI = doc->getRoot();
    if (playerInventory) {
        openInventory();
    } else {
        inventoryOpen = true;
    }
    add(HudElement(hud_element_mode::inventory_bound, doc, secondUI, false));
}

void Hud::openPermanent(UiDocument* doc) {
    auto root = doc->getRoot();
    remove(root);

    auto invview = std::dynamic_pointer_cast<InventoryView>(root);
    if (invview) {
        auto inventory = player->getInventory();
        invview->bind(inventory, frontend, interaction.get());
    }
    add(HudElement(hud_element_mode::permanent, doc, doc->getRoot(), false));
}

void Hud::closeInventory() {
    inventoryOpen = false;
    ItemStack& grabbed = interaction->getGrabbedItem();
    grabbed.clear();
    inventoryView = nullptr;
    blockUI = nullptr;
    secondUI = nullptr;
}

void Hud::add(HudElement element) {
    gui->add(element.getNode());
    auto invview = std::dynamic_pointer_cast<InventoryView>(element.getNode());
    auto document = element.getDocument();
    if (document) {
        if (invview) {
            auto inventory = invview->getInventory();
            scripting::on_ui_open(
                element.getDocument(), 
                inventory.get(), 
                blockPos
            );
        } else {
            scripting::on_ui_open(
                element.getDocument(), 
                nullptr, 
                blockPos
            );
        }
    }
    elements.push_back(element);
}

void Hud::onRemove(HudElement& element) {
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

void Hud::remove(std::shared_ptr<gui::UINode> node) {
    for (auto& element : elements) {
        if (element.getNode() == node) {
            element.setRemoved();
            onRemove(element);
        }
    }
    cleanup();
}

void Hud::draw(const GfxContext& ctx){
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
        GfxContext chctx = ctx.sub();
        chctx.setBlendMode(blendmode::inversion);
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
                glm::min(width/2-invwidth/2, width-caWidth-10-invwidth),
                height/2-totalHeight/2
            ));
        }
    }
    grabbedItemView->setPos(glm::vec2(Events::cursor));
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
    
    auto menu = gui->getMenu();
    if (pause) {
        menus::create_pause_panel(engine, frontend->getController());
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
