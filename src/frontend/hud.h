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
class ContentGfxCache;
class BlocksPreview;
class InventoryView;

namespace gui {
	class GUI;
	class UINode;
}

class HudRenderer {
	Level* level;
    Assets* assets;
	Camera* uicamera;
	BlocksPreview* blocksPreview;

	int fps = 60;
	int fpsMin = 60;
	int fpsMax = 60;
	std::wstring fpsString;
	bool inventoryOpen = false;
	bool pause = false;

    std::unique_ptr<InventoryView> contentAccess;
	std::shared_ptr<gui::UINode> debugPanel;
	gui::GUI* gui;
	const ContentGfxCache* const cache;

    void createDebugPanel(Engine* engine);
public:
	HudRenderer(Engine* engine, 
				Level* level, 
				const ContentGfxCache* cache);
	~HudRenderer();

	void update();
	void draw(const GfxContext& context);
	void drawDebug(int fps);

	bool isInventoryOpen() const;
	bool isPause() const;
};

#endif /* SRC_HUD_H_ */
