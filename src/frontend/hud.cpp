#include "hud.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <assert.h>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
#include "../voxels/Chunks.h"
#include "../voxels/Block.h"
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

using namespace gui;

static std::shared_ptr<Label> create_label(wstringsupplier supplier) {
    auto label = std::make_shared<Label>(L"-");
    label->textSupplier(supplier);
    return label;
}

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

std::shared_ptr<UINode> Hud::createDebugPanel(Engine* engine) {
    auto level = frontend->getLevel();

    auto panel = std::make_shared<Panel>(glm::vec2(250, 200), glm::vec4(5.0f), 2.0f);
    panel->listenInterval(0.5f, [this]() {
        fpsString = std::to_wstring(fpsMax)+L" / "+std::to_wstring(fpsMin);
        fpsMin = fps;
        fpsMax = fps;
    });
    panel->setCoord(glm::vec2(10, 10));
    panel->add(create_label([this](){ return L"fps: "+this->fpsString;}));
    panel->add(create_label([](){
        return L"meshes: " + std::to_wstring(Mesh::meshesCount);
    }));
    panel->add(create_label([=](){
        auto& settings = engine->getSettings();
        bool culling = settings.graphics.frustumCulling;
        return L"frustum-culling: "+std::wstring(culling ? L"on" : L"off");
    }));
    panel->add(create_label([=]() {
        return L"chunks: "+std::to_wstring(level->chunks->chunksCount)+
               L" visible: "+std::to_wstring(level->chunks->visible);
    }));
    panel->add(create_label([=](){
        auto player = level->player;
        auto* indices = level->content->getIndices();
        auto def = indices->getBlockDef(player->selectedVoxel.id);
        std::wstringstream stream;
        stream << std::hex << level->player->selectedVoxel.states;
        if (def) {
            stream << L" (" << util::str2wstr_utf8(def->name) << L")";
        }
        return L"block: "+std::to_wstring(player->selectedVoxel.id)+
               L" "+stream.str();
    }));
    panel->add(create_label([=](){
        return L"seed: "+std::to_wstring(level->world->getSeed());
    }));

    for (int ax = 0; ax < 3; ax++) {
        auto sub = std::make_shared<Container>(glm::vec2(), glm::vec2(250, 27));

        std::wstring str = L"x: ";
        str[0] += ax;
        auto label = std::make_shared<Label>(str);
        label->setMargin(glm::vec4(2, 3, 2, 3));
        label->setSize(glm::vec2(20, 27));
        sub->add(label);
        sub->setColor(glm::vec4(0.0f));

        // Coord input
        auto box = std::make_shared<TextBox>(L"");
        box->setTextSupplier([=]() {
            Hitbox* hitbox = level->player->hitbox.get();
            return util::to_wstring(hitbox->position[ax], 2);
        });
        box->setTextConsumer([=](std::wstring text) {
            try {
                glm::vec3 position = level->player->hitbox->position;
                position[ax] = std::stoi(text);
                level->player->teleport(position);
            } catch (std::invalid_argument& _){
            }
        });
        box->setOnEditStart([=](){
            Hitbox* hitbox = level->player->hitbox.get();
            box->setText(std::to_wstring(int(hitbox->position[ax])));
        });
        box->setSize(glm::vec2(230, 27));

        sub->add(box, glm::vec2(20, 0));
        panel->add(sub);
    }
    panel->add(create_label([=](){
        int hour, minute, second;
        timeutil::from_value(level->world->daytime, hour, minute, second);

        std::wstring timeString = 
                     util::lfill(std::to_wstring(hour), 2, L'0') + L":" +
                     util::lfill(std::to_wstring(minute), 2, L'0');
        return L"time: "+timeString;
    }));
    {
        auto bar = std::make_shared<TrackBar>(0.0f, 1.0f, 1.0f, 0.005f, 8);
        bar->setSupplier([=]() {return level->world->daytime;});
        bar->setConsumer([=](double val) {level->world->daytime = val;});
        panel->add(bar);
    }
    {
        auto bar = std::make_shared<TrackBar>(0.0f, 1.0f, 0.0f, 0.005f, 8);
        bar->setSupplier([=]() {return WorldRenderer::fog;});
        bar->setConsumer([=](double val) {WorldRenderer::fog = val;});
        panel->add(bar);
    }
    {
        auto checkbox = std::make_shared<FullCheckBox>(
            L"Show Chunk Borders", glm::vec2(400, 24)
        );
        checkbox->setSupplier([=]() {
            return engine->getSettings().debug.showChunkBorders;
        });
        checkbox->setConsumer([=](bool checked) {
            engine->getSettings().debug.showChunkBorders = checked;
        });
        panel->add(checkbox);
    }
    panel->refresh();
    return panel;
}

std::shared_ptr<InventoryView> Hud::createContentAccess() {
    auto level = frontend->getLevel();
    auto content = level->content;
    auto indices = content->getIndices();
    auto player = level->player;
    auto inventory = player->getInventory();
    
    int itemsCount = indices->countItemDefs();
    auto accessInventory = std::make_shared<Inventory>(0, itemsCount);
    for (int id = 1; id < itemsCount; id++) {
        accessInventory->getSlot(id-1).set(ItemStack(id, 1));
    }

    SlotLayout slotLayout(-1, glm::vec2(), false, true, 
    [=](uint, ItemStack& item) {
        auto copy = ItemStack(item);
        inventory->move(copy, indices);
    }, 
    [=](ItemStack& item, ItemStack& grabbed) {
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
    auto level = frontend->getLevel();
    auto player = level->player;
    auto inventory = player->getInventory();

    SlotLayout slotLayout(-1, glm::vec2(), false, false, nullptr, nullptr);
    InventoryBuilder builder;
    builder.addGrid(10, 10, glm::vec2(), 4, true, slotLayout);
    auto view = builder.build();

    view->setOrigin(glm::vec2(view->getSize().x/2, 0));
    view->bind(inventory, frontend, interaction.get());
    view->setInteractive(false);
    return view;
}

Hud::Hud(Engine* engine, LevelFrontend* frontend) 
    : assets(engine->getAssets()), 
      gui(engine->getGUI()),
      frontend(frontend)
{
    auto menu = gui->getMenu();

    interaction = std::make_unique<InventoryInteraction>();
    grabbedItemView = std::make_shared<SlotView>(
        SlotLayout(-1, glm::vec2(), false, false, nullptr, nullptr)
    );
    grabbedItemView->bind(
        interaction->getGrabbedItem(), 
        frontend, 
        interaction.get()
    );
    grabbedItemView->setColor(glm::vec4());
    grabbedItemView->setInteractive(false);
    grabbedItemView->setZIndex(1);

    contentAccess = createContentAccess();
    contentAccessPanel = std::make_shared<Panel>(
        contentAccess->getSize(), glm::vec4(0.0f), 0.0f
    );
    contentAccessPanel->setColor(glm::vec4());
    contentAccessPanel->add(contentAccess);
    contentAccessPanel->setScrollable(true);

    hotbarView = createHotbar();
    darkOverlay = std::make_unique<Panel>(glm::vec2(4000.0f));
    darkOverlay->setColor(glm::vec4(0, 0, 0, 0.5f));
    darkOverlay->setZIndex(-1);
    darkOverlay->setVisible(false);

    uicamera = std::make_unique<Camera>(glm::vec3(), 1);
    uicamera->perspective = false;
    uicamera->flipped = true;

    debugPanel = createDebugPanel(engine);
    menu->reset();

    debugPanel->setZIndex(2);
    
    gui->add(darkOverlay);
    gui->add(hotbarView);
    gui->add(debugPanel);
    gui->add(contentAccessPanel);
    gui->add(grabbedItemView);
}

Hud::~Hud() {
    // removing all controlled ui
    gui->remove(grabbedItemView);
    for (auto& element : elements) {
        remove(element);
    }
    gui->remove(hotbarView);
    gui->remove(darkOverlay);
    gui->remove(contentAccessPanel);
    gui->remove(debugPanel);
}

void Hud::drawDebug(int fps){
    this->fps = fps;
    fpsMin = min(fps, fpsMin);
    fpsMax = max(fps, fpsMax);
}

/**
 * Remove all elements marked as removed
 */
void Hud::cleanup() {
    auto it = std::remove_if(elements.begin(), elements.end(), [](const HudElement& e) {
        return e.isRemoved();
    });
    elements.erase(it, elements.end());
} 

void Hud::update(bool visible) {
    auto level = frontend->getLevel();
    auto player = level->player;
    auto menu = gui->getMenu();

    debugPanel->setVisible(player->debug && visible);

    if (!visible && inventoryOpen) {
        closeInventory();
    }
    if (pause && menu->getCurrent().panel == nullptr) {
        setPause(false);
    }
    if (Events::jpressed(keycode::ESCAPE) && !gui->isFocusCaught()) {
        if (pause) {
            setPause(false);
        } else if (inventoryOpen) {
            closeInventory();
        } else {
            setPause(true);
        }
    }
    if (visible && Events::jactive(BIND_HUD_INVENTORY)) {
        if (!pause) {
            if (inventoryOpen) {
                closeInventory();
            } else {
                openInventory();
            }
        }
    }
    if ((pause || inventoryOpen) == Events::_cursor_locked) {
        Events::toggleCursor();
    }

    glm::vec2 invSize = contentAccessPanel->getSize();
    contentAccessPanel->setVisible(inventoryOpen);
    contentAccessPanel->setSize(glm::vec2(invSize.x, Window::height));
    contentAccess->setMinSize(glm::vec2(1, Window::height));
    // hotbarView->setVisible(visible && !inventoryOpen);

    for (int i = keycode::NUM_1; i <= keycode::NUM_9; i++) {
        if (Events::jpressed(i)) {
            player->setChosenSlot(i - keycode::NUM_1);
        }
    }
    if (Events::jpressed(keycode::NUM_0)) {
        player->setChosenSlot(9);
    }
    if (!pause && !inventoryOpen && Events::scroll) {
        int slot = player->getChosenSlot();
        slot = (slot - Events::scroll) % 10;
        if (slot < 0) {
            slot += 10;
        }
        player->setChosenSlot(slot);
    }

    for (auto& element : elements) {
        element.update(pause, inventoryOpen, player->debug);
        if (element.isRemoved()) {
            remove(element);
        }
    }
    cleanup();
}

/** 
 * Show inventory on the screen and turn on inventory mode blocking movement
 */
void Hud::openInventory() {
    auto level = frontend->getLevel();
    auto player = level->player;
    auto inventory = player->getInventory();

    inventoryOpen = true;

    auto inventoryDocument = assets->getLayout("core:inventory");
    inventoryView = std::dynamic_pointer_cast<InventoryView>(inventoryDocument->getRoot());
    inventoryView->bind(inventory, frontend, interaction.get());
    add(HudElement(hud_element_mode::inventory_bound, inventoryDocument, inventoryView, false));
}

/**
 * Show player inventory + block UI
 * @param block world position of the open block
 * @param doc block UI document (root element must be an InventoryView)
 * @param blockinv block inventory. 
 * In case of nullptr a new virtual inventory will be created
 */
void Hud::openInventory(glm::ivec3 block, UiDocument* doc, std::shared_ptr<Inventory> blockinv) {
    auto level = frontend->getLevel();
    blockUI = std::dynamic_pointer_cast<InventoryView>(doc->getRoot());
    if (blockUI == nullptr) {
        throw std::runtime_error("block UI root element must be 'inventory'");
    }
    openInventory();
    if (blockinv == nullptr) {
        Events::toggleCursor();
        abort();
        blockinv = level->inventories->createVirtual(blockUI->getSlotsCount());
    }
    blockUI->bind(blockinv, frontend, interaction.get());
    currentblock = block;
    add(HudElement(hud_element_mode::inventory_bound, doc, blockUI, false));
}

/** 
 * Add element as permanent overlay
 * @param doc element layout document
 */
void Hud::openPermanent(UiDocument* doc) {
    remove(doc->getRoot());
    add(HudElement(hud_element_mode::permanent, doc, doc->getRoot(), false));
}

/**
 * Hide inventory and turn off inventory mode
 */
void Hud::closeInventory() {
    auto level = frontend->getLevel();

    inventoryOpen = false;
    ItemStack& grabbed = interaction->getGrabbedItem();
    grabbed.clear();
    inventoryView = nullptr;
    if (blockUI) {
        auto blockinv = blockUI->getInventory();
        // todo: do it automatically
        if (blockinv->isVirtual()) {
            level->inventories->remove(blockinv->getId());   
        }
        blockUI = nullptr;
    }
}

void Hud::add(HudElement element) {
    gui->add(element.getNode());
    auto invview = std::dynamic_pointer_cast<InventoryView>(element.getNode());
    auto document = element.getDocument();
    if (document) {
        if (invview) {
            auto inventory = invview->getInventory();
            scripting::on_ui_open(element.getDocument(), inventory.get());
        } else {
            scripting::on_ui_open(element.getDocument(), nullptr);
        }
    }
    elements.push_back(element);
}

void Hud::remove(HudElement& element) {
    auto document = element.getDocument();
    if (document) {
        Inventory* inventory = nullptr;
        auto invview = std::dynamic_pointer_cast<InventoryView>(element.getNode());
        if (invview) {
            inventory = invview->getInventory().get();
        }
        scripting::on_ui_close(document, inventory);
    }
    gui->remove(element.getNode());
}

// todo: refactor this garbage
void Hud::remove(std::shared_ptr<UINode> node) {
    for (auto& element : elements) {
        if (element.getNode() == node) {
            element.setRemoved();
            remove(element);
        }
    }
    cleanup();
}

void Hud::draw(const GfxContext& ctx){
    auto level = frontend->getLevel();
    auto player = level->player;

    const Viewport& viewport = ctx.getViewport();
    const uint width = viewport.getWidth();
    const uint height = viewport.getHeight();

    uicamera->setFov(height);

    auto batch = ctx.getBatch2D();
    batch->begin();

    Shader* uishader = assets->getShader("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());
    
    hotbarView->setCoord(glm::vec2(width/2, height-65));
    hotbarView->setSelected(player->getChosenSlot());

    // Crosshair
    if (!pause && Events::_cursor_locked && !level->player->debug) {
        GfxContext chctx = ctx.sub();
        chctx.blendMode(blendmode::inversion);
        auto texture = assets->getTexture("gui/crosshair");
        batch->texture(texture);
        int chsizex = texture != nullptr ? texture->width : 16;
        int chsizey = texture != nullptr ? texture->height : 16;
        batch->rect(
            (width-chsizex)/2, (height-chsizey)/2, 
            chsizex, chsizey, 0,0, 1,1, 1,1,1,1
        );
        batch->render();
    }

    // Delta-time visualizer
    if (level->player->debug) {
        batch->texture(nullptr);
        const int dmwidth = 256;
        const float dmscale = 4000.0f;
        static float deltameter[dmwidth]{};
        static int index=0;
        index = index + 1 % dmwidth;
        deltameter[index%dmwidth] = glm::min(0.2f, 1.f/fps)*dmscale;
        batch->lineWidth(1);
        for (int i = index+1; i < index+dmwidth; i++) {
            int j = i % dmwidth;
            batch->line(width-dmwidth+i-index, height-deltameter[j], 
                        width-dmwidth+i-index, height, 1.0f, 1.0f, 1.0f, 0.2f);
        }
    }

    if (inventoryOpen) {
        float caWidth = contentAccess->getSize().x;
        contentAccessPanel->setCoord(glm::vec2(width-caWidth, 0));

        glm::vec2 invSize = inventoryView->getSize();
        inventoryView->setCoord(glm::vec2(
            glm::min(width/2-invSize.x/2, width-caWidth-10-invSize.x), 
            height/2-invSize.y/2
        ));
    }
    grabbedItemView->setCoord(glm::vec2(Events::cursor));
    batch->render();
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
        menu->setPage("pause");
    } else {
        menu->reset();
    }
    darkOverlay->setVisible(pause);
    menu->setVisible(pause);
}
