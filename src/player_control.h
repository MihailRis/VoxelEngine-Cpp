#ifndef PLAYER_CONTROL_H_
#define PLAYER_CONTROL_H_

class PhysicsSolver;
class Chunks;
class Player;
class LineBatch;
class Level;

void update_controls(PhysicsSolver* physics, Chunks* chunks, Player* player, float delta);
void update_interaction(Level* level, LineBatch* lineBatch);

#endif /* PLAYER_CONTROL_H_ */
