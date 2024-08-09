#pragma once

#include <string>

inline const std::string CORE_EMPTY = "core:empty";
inline const std::string CORE_AIR = "core:air";

inline const std::string TEXTURE_NOTFOUND = "notfound";

// built-in bindings
inline const std::string BIND_DEVTOOLS_CONSOLE = "devtools.console";
inline const std::string BIND_CHUNKS_RELOAD = "chunks.reload";
inline const std::string BIND_MOVE_FORWARD = "movement.forward";
inline const std::string BIND_MOVE_BACK = "movement.back";
inline const std::string BIND_MOVE_LEFT = "movement.left";
inline const std::string BIND_MOVE_RIGHT = "movement.right";
inline const std::string BIND_MOVE_JUMP = "movement.jump";
inline const std::string BIND_MOVE_SPRINT = "movement.sprint";
inline const std::string BIND_MOVE_CROUCH = "movement.crouch";
inline const std::string BIND_MOVE_CHEAT = "movement.cheat";
inline const std::string BIND_CAM_ZOOM = "camera.zoom";
inline const std::string BIND_CAM_MODE = "camera.mode";
inline const std::string BIND_PLAYER_NOCLIP = "player.noclip";
inline const std::string BIND_PLAYER_FLIGHT = "player.flight";
inline const std::string BIND_PLAYER_ATTACK = "player.attack";
inline const std::string BIND_PLAYER_BUILD = "player.build";
inline const std::string BIND_PLAYER_PICK = "player.pick";
inline const std::string BIND_HUD_INVENTORY = "hud.inventory";

class EnginePaths;
class ContentBuilder;

namespace corecontent {
    void setup(EnginePaths* paths, ContentBuilder* builder);
}
