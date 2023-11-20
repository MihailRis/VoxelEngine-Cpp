#ifndef SRC_HUD_H_
#define SRC_HUD_H_

#include <string>
#include <memory>

#include "../graphics/GfxContext.h"

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;
class Level;
class Engine;

namespace gui {
	class GUI;
	class UINode;
}

class HudRenderer {
	Level* level;
    Assets* assets;
	Batch2D* batch;
	Camera* uicamera;

	int fps = 60;
	int fpsMin = 60;
	int fpsMax = 60;
	std::wstring fpsString;
	bool occlusion;
	bool inventoryOpen = false;
	bool pause = false;

	std::shared_ptr<gui::UINode> debugPanel;
	gui::GUI* gui;
public:
	HudRenderer(Engine* engine, Level* level);
	~HudRenderer();

	void update();
	void drawInventory(const GfxContext& ctx, Player* player);
	void draw(const GfxContext& context);
	void drawDebug(int fps, bool occlusion);

	bool isInventoryOpen() const;
	bool isPause() const;
};

#endif /* SRC_HUD_H_ */
