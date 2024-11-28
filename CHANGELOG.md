# 0.25 - 2024.11.29

[Documentation](https://github.com/MihailRis/VoxelEngine-Cpp/tree/release-0.25/doc/en/main-page.md) for 0.25

Table of contents:

- [Added](#added)
    - [Functions](#functions)
- [Fixes](#fixes)

## Added

- 3dtext
- blockwraps
- network (http requests and sockets)
- libraries:
	- base64
	- gfx.text3d
	- gfx.blockwraps
    - network
- events:
    - on_replaced
    - on_block_replaced
- structures 'lowering' property
- add 'hint' property to textbox
- add 'taking' and 'placing' properties to slot and slotsgrid
- add 'scroll-step' property to container
- add 'line-numbers' and 'text-color' to textbox
- modules:
    - base:util
- uinode property 'id'
- block.materials table
- block.properties table
- item.properties table
- add version to world info table
- add 'sizeSpread' particles property

### Functions

- player.is_infinite_items
- player.set_infinite_items
- player.is_instant_destruction
- player.set_instant_destruction
- player.get_name
- player.set_name
- hud.open
- base64.encode
- base64.decode
- utf8.escape
- string.escape
- textbox:lineAt
- textbox:linePos
- network.get
- network.get_binary
- network.tcp_connect
- network.tcp_open
- network.get_total_upload
- network.get_total_download
- gfx.text3d.show
- gfx.text3d.hide
- gfx.text3d.get_text
- gfx.text3d.set_text
- gfx.text3d.get_pos
- gfx.text3d.set_pos
- gfx.text3d.get_axis_x
- gfx.text3d.set_axis_x
- gfx.text3d.get_axis_y
- gfx.text3d.set_axis_y
- gfx.text3d.set_rotation
- gfx.text3d.update_settings

## Fixes

- [fix translucent blocks render](https://github.com/MihailRis/VoxelEngine-Cpp/pull/370)
- [fix blocks selection with semi-transparent blocks](https://github.com/MihailRis/VoxelEngine-Cpp/commit/171cbb48d099032d7e78c51a46c374104f96f0d1)
- [fix: commands repository not reset before world open](https://github.com/MihailRis/VoxelEngine-Cpp/commit/1a00a91b604399f3108aa995422d371e573e650b)
- [mip-mapping related fixes](https://github.com/MihailRis/VoxelEngine-Cpp/commit/d9277e1b31714632bd7f5f601b8362a9e7cb8819)
- [fix disabled slots display](https://github.com/MihailRis/VoxelEngine-Cpp/commit/e8ee3e04b1398a3ada8445591267525304410571)
- [fix attack](https://github.com/MihailRis/VoxelEngine-Cpp/commit/bc17abc8b3ee7ff9027f7e3c375ca0330bb8e7bc)
- [fix: commands repository not reset before world open](https://github.com/MihailRis/VoxelEngine-Cpp/commit/1a00a91b604399f3108aa995422d371e573e650b)
- [fix stdlib.lua](https://github.com/MihailRis/VoxelEngine-Cpp/commit/6ec33ab98c78523eaececf40f113f2323d25a33a)
- [fix file.write_bytes](https://github.com/MihailRis/VoxelEngine-Cpp/commit/0fec17a8b69ac81255b77022f3af5addf8fcc8f8)
- [fix World::nextInventoryId](https://github.com/MihailRis/VoxelEngine-Cpp/commit/371fdaedcef2c163edd226160f388068b2bf5e83)
- [fix block inventory unbinding](https://github.com/MihailRis/VoxelEngine-Cpp/commit/6f6c2a916afd6b9b79221111fc72b1a86109be13)
- [fix xml text escapes handling](https://github.com/MihailRis/VoxelEngine-Cpp/commit/53c54dc91d132c221ff5fea2f7e9fb4568db9a0f)
- [fix `\'` escape parsing](https://github.com/MihailRis/VoxelEngine-Cpp/commit/2bc6cbda2e809b14fa6cffe09161b53c1636675f)
- [fix crosshair look](https://github.com/MihailRis/VoxelEngine-Cpp/commit/e034bda477c35efe96548e78ecc722966a7a2197)
- [fix: actual block inventory size not updating on inventory-size property update](https://github.com/MihailRis/VoxelEngine-Cpp/commit/1ba5b0ce33103e539ccb199ee1cd52095e286a1f)
- [fix falling block hitbox](https://github.com/MihailRis/VoxelEngine-Cpp/commit/352ef6485a4b796d1cdc8dd0e00ab1a1d72a2c0a)
- [fix console position](https://github.com/MihailRis/VoxelEngine-Cpp/commit/3ea213e8d3cee7be55ec39ffb18dc557dec7557b)
- [fix: fatal error on pack removal when no world open](https://github.com/MihailRis/VoxelEngine-Cpp/commit/78d5ab02c2ba8a3d05cf5639eb10a49c9ca14ec3)
- [fix custom model lighting](https://github.com/MihailRis/VoxelEngine-Cpp/commit/a333cadfcaeb485a30833343d55faf01b28a5c5f)
- [fix: emitter does not skip particles](https://github.com/MihailRis/VoxelEngine-Cpp/commit/983e516fb4ebc1f2def592f2b7f3195d968deed2)
- [fix old custom models render](https://github.com/MihailRis/VoxelEngine-Cpp/commit/82733d38011b52a426cb74560521949c1cd43cc1)
