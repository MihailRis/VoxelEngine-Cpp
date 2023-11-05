#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;

class HudRenderer {
    Assets* assets;
	Batch2D* batch;
	Camera* uicamera;
public:
	HudRenderer(Assets* assets);
	~HudRenderer();
	void drawInventory(Player* player);
	void draw(Level* level);
	void drawDebug(Level* level, int fps, bool occlusion);
};

#endif /* SRC_HUD_RENDER_H_ */
