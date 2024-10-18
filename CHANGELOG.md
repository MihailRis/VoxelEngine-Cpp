# 0.23 - 2024.10.19

[Documentation](https://github.com/MihailRis/VoxelEngine-Cpp/tree/release-0.23/doc/en/main-page.md) for 0.23

Table of contents:

- [Added](#added)
    - [Functions](#functions)
- [Changes](#changes)
- [Fixes](#fixes)

## Added

- world generation engine instead of hardcoded generator
- world generators
    - core:default
    - base:demo
- block fields (metadata)
- resource aliases (resource-aliases.json)
    - cameras
- libraries
    - generation
    - bjson
- commands:
	- fragment.save
	- fragment.crop
- blocks:
	- core:obstacle
	- core:struct_air
	- base:coal_ore
- settings:
	- graphics.chunk-max-vertices
	- graphics.chunk-max-renderers
- block properties:
	- surface-replacement
	- fields
- 'parent' property for blocks, items and entities
- filesystem entry points:
	- config
	- export
- lua usertypes:
	- Heightmap
	- VoxelFragment
- raycast filter
- (project) add unit tests framework (gtest)
- (project) change project title to VoxelCore

### Functions

- debug.print
- pack.shared_file
- block.get_field
- block.set_field
- item.caption
- file.read_combined_list
- cameras.get(int)
- bjson.tobytes
- bjson.frombytes
- generation.create_fragment
- generation.load_fragment
- generation.save_fragment
- generation.get_default_generator
- generation.get_generators
- uinode:getContentOffset

## Changes

- upgrade world regions format
- upgrade toml parser to 1.0.0 support
- json.tostring now accepts any supported value
- json.parse now accepts any supported value as root element

## Fixes

- [fix: extended block always main segment passed to on_iteract](https://github.com/MihailRis/VoxelEngine-Cpp/commit/fbca439b2da5a236a122c29488dc8809044ae919)
- [fix: backlight setting not applying on change](https://github.com/MihailRis/VoxelEngine-Cpp/commit/d59fac61bb5ae5949b49f10ac71c22b595dcdff7 "fix: backlight setting not applying on change")
- [fix: backlight not applied to entities](https://github.com/MihailRis/VoxelEngine-Cpp/commit/45a1e1df82967141dfb6d4b9b298deb4dfbf44c0 "fix: backlight not applied to entities")
- [fix: extended block always main segment passed to on_iteract](https://github.com/MihailRis/VoxelEngine-Cpp/commit/fbca439b2da5a236a122c29488dc8809044ae919 "fix: extended block always main segment passed to on_iteract")
- [fix block.get_hitbox with non rotatable blocks](https://github.com/MihailRis/VoxelEngine-Cpp/commit/b9074ebe4788d0016a9fd7563b59816b6300c06d "fix block.get_hitbox with non rotatable blocks")
- [fix: entity shading is incorrect when it is upper than max height](https://github.com/MihailRis/VoxelEngine-Cpp/commit/45a793d6475b4d5b7c59e9c18492aa45767e2236 "fix: entity shading is incorrect when it is upper than max height")
- [fix: toggle fullscreen GLFW invalid enum error](https://github.com/MihailRis/VoxelEngine-Cpp/commit/85bea6f17dc7815569a28e70423b704c476ed410 "fix: toggle fullscreen GLFW invalid enum error")
- [fix: flight can stop on noclip enabled](https://github.com/MihailRis/VoxelEngine-Cpp/commit/f63ab345eaaf7885cfd0298a99cea58a423741fb "fix: flight can stop on noclip enabled")
- [fix: block model "x" preview](https://github.com/MihailRis/VoxelEngine-Cpp/pull/300)
- [Batch3D::point() buffer overflow](https://github.com/MihailRis/VoxelEngine-Cpp/pull/302)
- [fix player entity teleport using debug_panel](https://github.com/MihailRis/VoxelEngine-Cpp/commit/ba9417a7e4638a3b09568895e4af5b702da80c16)
- fix fatal animator error
