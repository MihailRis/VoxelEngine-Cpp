# 0.24 - 2024.11.07

[Documentation](https://github.com/MihailRis/VoxelEngine-Cpp/tree/release-0.24/doc/en/main-page.md) for 0.24

Table of contents:

- [Added](#added)
    - [Functions](#functions)
- [Changes](#changes)
- [Fixes](#fixes)

## Added

- particles
- VEC3 models support
- handhold item display
- rules
- events:
  - on_block_broken (documented)
  - on_block_placed (documented)
  - on_block_interact
- libraries:
  - utf8
  - rules
- bindings:
  - player.destroy
  - player.fast_interaction
- water overlay
- block models from OBJ or VEC3
- bicubic heightmaps interpolation method
- unicode escapes support
- fragments placements
- console commands:
  - time.daycycle
  - fragment.place
  - rule.list
  - rule.set
- text field 'subconsumer'
- shader uniforms:
  - u_lightDir to main shader
  - u_dayTime to skybox shader
- block properties:
  - overlay-texture
  - model-name
- item properties:
  - model-name
- 'Open content folder' buttons
- 'Background framerate limit' setting

### Functions

- core.open_folder
- world.get_generator
- world.is_open
- item.placing_block
- item.model_name
- item.emission
- entities.get_hitbox
- utf8.tobytes
- utf8.tostring
- utf8.length
- utf8.codepoint
- utf8.encode
- utf8.sub
- utf8.upper
- utf8.lower
- file.read_combined_object
- fragment:place
- rules.create
- rules.listen
- rules.unlisten
- rules.get
- rules.set
- rules.reset
- input.set_enabled
- hud._is_content_access
- hud._set_content_access
- hud._set_debug_cheats

Documented:
- file.read_combined_list
- file.list
- file.list_all_res
- input.is_active

## Changes

- major skybox optimization
- chunks-renderer optimization
- libspng replaced with libpng on Windows
- console commands:
  - blocks.fill
  - fragment.save
- added 'def' to core.get_setting_info tables
- water texture

## Fixes

- [fix fatal error on editing texbox not having any consumer](https://github.com/MihailRis/VoxelEngine-Cpp/commit/22fa082fc6299ffa3196d62c67e01b849c35b8eb)
- [fix commands boolean type support](https://github.com/MihailRis/VoxelEngine-Cpp/commit/a50cb109c8e3ca0f7a591bf126f07aee36c962e6)
- [fix potential null dereferences on incorrect block.* functions use](https://github.com/MihailRis/VoxelEngine-Cpp/commit/961773c9f9745c15eb8d697c1538ac8e21f24da3)
- [fix: draw-group not copied](https://github.com/MihailRis/VoxelEngine-Cpp/commit/dc8bad2af67e70b0b2346f516028e5795f597737)
- [fix: generator-providing pack may be removed](https://github.com/MihailRis/VoxelEngine-Cpp/commit/6f2f365278eb1866c773890471b7269a5ef45305)
- [fix colision check on block place](https://github.com/MihailRis/VoxelEngine-Cpp/commit/726ee8ad703bc57530b881450b8839aaec6b97c9)
- [fix collision detection bug](https://github.com/MihailRis/VoxelEngine-Cpp/commit/7fcc34ba4cf14097dfda26054b028c5e8771d26c)
- [fix: blocks lighting bug fix](https://github.com/MihailRis/VoxelEngine-Cpp/commit/9d3e872f88de2648f8c0f2e4611b30f5ce8999cf)
- [fix: inaccurate framerate limit on Windows](https://github.com/MihailRis/VoxelEngine-Cpp/commit/3f531bbf98da5ad751dce1220c5c5fdf35f86c92)
- [fix block.get_hitbox again](https://github.com/MihailRis/VoxelEngine-Cpp/commit/edad594101e5808ccf14e0edefedbe87cb8f983b)
- [fix string.replace](https://github.com/MihailRis/VoxelEngine-Cpp/commit/44fd5416a9a110a12f8b3f2d369e5638055b306e)
