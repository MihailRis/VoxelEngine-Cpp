#ifndef SRC_HUD_H_
#define SRC_HUD_H_

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "../graphics/GfxContext.h"

class Camera;
class Level;
class Block;
class Assets;
class Player;
class Level;
class Engine;
class SlotView;
class InventoryView;
class LevelFrontend;
class InventoryInteraction;

namespace gui {
	class GUI;
	class UINode;
    class Panel;
}

class HudRenderer {
    Assets* assets;
	Camera* uicamera;

	int fps = 60;
	int fpsMin = 60;
	int fpsMax = 60;
	std::wstring fpsString;
	bool inventoryOpen = false;
	bool pause = false;

    std::shared_ptr<gui::Panel> contentAccessPanel;
    std::shared_ptr<InventoryView> contentAccess;
    std::shared_ptr<InventoryView> hotbarView;
    std::shared_ptr<InventoryView> inventoryView;
	std::shared_ptr<gui::UINode> debugPanel;
    std::shared_ptr<gui::Panel> darkOverlay;
    std::unique_ptr<InventoryInteraction> interaction;
    std::shared_ptr<SlotView> grabbedItemView;
	gui::GUI* gui;
	LevelFrontend* frontend;

    void createDebugPanel(Engine* engine);

    std::shared_ptr<InventoryView> createContentAccess();
    std::shared_ptr<InventoryView> createHotbar();
    std::shared_ptr<InventoryView> createInventory();
public:
	HudRenderer(Engine* engine, LevelFrontend* frontend);
	~HudRenderer();

	void update(bool hudVisible);
	void draw(const GfxContext& context);
	void drawDebug(int fps);

	bool isInventoryOpen() const;
	bool isPause() const;

    void closeInventory();
};

#endif /* SRC_HUD_H_ */
