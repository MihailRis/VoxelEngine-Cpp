#ifndef PLAYER_CONTROL_H_
#define PLAYER_CONTROL_H_

class PhysicsSolver;
class Chunks;
class Player;
class Lighting;
class LineBatch;

void update_controls(PhysicsSolver* physics, Chunks* chunks, Player* player, float delta);
void update_interaction(Chunks* chunks, PhysicsSolver* physics, Player* player, Lighting* lighting, LineBatch* lineBatch);

#endif /* PLAYER_CONTROL_H_ */
