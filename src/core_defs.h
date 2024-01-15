#ifndef SRC_CORE_DEFS_H_
#define SRC_CORE_DEFS_H_

#include <string>


const std::string TEXTURE_NOTFOUND = "notfound";

/* bindings used in engine code */
const std::string BIND_MOVE_FORWARD = "movement.forward";
const std::string BIND_MOVE_BACK = "movement.back";
const std::string BIND_MOVE_LEFT = "movement.left";
const std::string BIND_MOVE_RIGHT = "movement.right";
const std::string BIND_MOVE_JUMP = "movement.jump";
const std::string BIND_MOVE_SPRINT = "movement.sprint";
const std::string BIND_MOVE_CROUCH = "movement.crouch";
const std::string BIND_MOVE_CHEAT = "movement.cheat";
const std::string BIND_CAM_ZOOM = "camera.zoom";
const std::string BIND_CAM_MODE = "camera.mode";
const std::string BIND_PLAYER_NOCLIP = "player.noclip";
const std::string BIND_PLAYER_FLIGHT = "player.flight";
const std::string BIND_PLAYER_ATTACK = "player.attack";
const std::string BIND_PLAYER_BUILD = "player.build";
const std::string BIND_PLAYER_PICK = "player.pick";
const std::string BIND_HUD_INVENTORY = "hud.inventory";

#endif // SRC_CORE_DEFS_H_