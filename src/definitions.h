#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <iostream>

#define BLOCK_AIR 0
#define BLOCK_DIRT 1
#define BLOCK_GRASS_BLOCK 2
#define BLOCK_LAMP 3
#define BLOCK_GLASS 4
#define BLOCK_PLANKS 5
#define BLOCK_WOOD 6
#define BLOCK_LEAVES 7
#define BLOCK_STONE 8
#define BLOCK_WATER 9
#define BLOCK_SAND 10
#define BLOCK_BEDROCK 11
#define BLOCK_GRASS 12
#define BLOCK_FLOWER 13
#define BLOCK_BRICK 14
#define BLOCK_METAL 15
#define BLOCK_RUST 16

#define BIND_MOVE_FORWARD "movement.forward"
#define BIND_MOVE_BACK "movement.back"
#define BIND_MOVE_LEFT "movement.left"
#define BIND_MOVE_RIGHT "movement.right"
#define BIND_MOVE_JUMP "movement.jump"
#define BIND_MOVE_SPRINT "movement.sprint"
#define BIND_MOVE_CROUCH "movement.crouch"
#define BIND_MOVE_CHEAT "movement.cheat"
#define BIND_CAM_ZOOM "camera.zoom"
#define BIND_PLAYER_NOCLIP "player.noclip"
#define BIND_PLAYER_FLIGHT "player.flight"
#define BIND_HUD_INVENTORY "hud.inventory"

extern void setup_bindings();
extern void setup_definitions();

#endif // DECLARATIONS_H

