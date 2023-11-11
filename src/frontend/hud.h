#ifndef SRC_HUD_H_
#define SRC_HUD_H_

#include <string>

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;
class Level;

namespace gui {
	class GUI;
	class Panel;
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
	bool pause = true;

	gui::Panel* debugPanel;
	gui::Panel* pauseMenu;
	gui::GUI* guiController;
public:
	HudRenderer(gui::GUI* gui, Level* level, Assets* assets);
	~HudRenderer();
	void drawInventory(Player* player);
	void draw();
	void drawDebug(int fps, bool occlusion);

	bool isInventoryOpen() const;
	bool isPause() const;
};

#endif /* SRC_HUD_H_ */
