#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

class Batch2D;
class Camera;
class World;
class Level;
class Assets;

class HudRenderer {
	Batch2D *batch;
	Camera *uicamera;
public:
	HudRenderer();
	~HudRenderer();
	void draw(World* world, Level* level, Assets* assets, bool devdata, int fps);
};

#endif /* SRC_HUD_RENDER_H_ */
