#ifndef SRC_HUD_H_
#define SRC_HUD_H_

#include <string>
#include <memory>

#include "../graphics/GfxContext.h"

namespace vulkan {
	class Batch2D;
	class WorldRenderer;
}

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;
class Level;
class Engine;
class ContentGfxCache;
class WorldRenderer;

namespace gui {
	class GUI;
	class UINode;
}

class HudRenderer {
	Level* level;
    Assets* assets;
	vulkan::Batch2D* batch;
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
	const ContentGfxCache* const cache;
	vulkan::WorldRenderer* renderer;
public:
	HudRenderer(Engine* engine, 
				Level* level, 
				const ContentGfxCache* cache, 
				vulkan::WorldRenderer* renderer);
	~HudRenderer();

	void update();
	void drawContentAccess(const GfxContext& ctx, Player* player);
	void draw(const GfxContext& context);
	void drawDebug(int fps, bool occlusion);

	bool isInventoryOpen() const;
	bool isPause() const;
};

#endif /* SRC_HUD_H_ */
