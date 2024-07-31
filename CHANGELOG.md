# 0.22 - 2024.08.01

[Documentation](https://github.com/MihailRis/VoxelEngine-Cpp/tree/release-0.22/doc/en/main-page.md) for 0.22

Table of contents:

- [Added](#added)
    - [Libraries](#libraries)
    - [Functions](#functions)
- [Changes](#changes)
- [Fixes](#fixes)

## Added

- entities, components
    - base:drop - dropped item
    - base:falling_block - falling block
    - base:player - player
- extended blocks
- resources (resources.json)
    - cameras
- libraries
    - cameras - cameras
    - entities - entities
    - vec2, vec3, vec4 - vectors
    - mat4 - 4x4 matrices
    - quat - quaternions
- skeletons (see doc/\*/rigging.md)
- models (only .obj is supported)
- block: base:door
- use of textures from atlases (atlas_name:texture_name) in image, entity models
- lists added to preload.json: atlas, model
- shadeless and ambient-occlusion properties for blocks
- setting: camera inertia
- event on_hud_render
- content pack menu
- "Display" section in settings
- framerate setting
- new documentation sections:
    - rigging
    - resources
    - entity-properties
    - scripting/
        - ecs
        - events
        - libcameras
        - libentities
        - libmat4
        - libquat
        - libvecn
- new sounds
- *change-on-release* and *sub-supplier* trackbar properties
- window icon

### Functions

- debug.log
- debug.warning
- debug.error
- input.is_pressed
- input.is_active
- hud.is_paused
- hud.is_inventory_open
- player.get_spawnpoint
- player.set_spawnpoint
- player.get_selected_block
- player.get_selected_entity
- player.get_entity
- player.get_camera
- player.set_camera
- block.place
- block.destruct
- block.get_picking_item
- block.raycast
- block.get_rotation_profile
- block.get_textures
- block.get_model
- block.get_hitbox
- block.is_extended
- block.get_size
- block.is_segment
- block.seek_origin
- block.compose_sate
- block.decompose_state
- math.clamp
- math.rand
- table.copy
- table.count_pairs
- table.random
- string.pattern_safe
- string.explode
- string.split
- string.formatted_time
- string.replace
- string.trim
- string.trim_right
- string.trim_left
- string.starts_with
- string.ends_with

###  Commands

- clear
- player.respawn
- entity.despawn
- time.uptime

## Changes

- content folder is now created automatically
- content error messages are now more detailed
- lua error messages now contain a call stack traceback
- updated documentation structure
- legacy functions (load_script, dofile) now generate warnings in the console with the call stack displayed
- some sounds of footsteps, destruction/installation of blocks
- removed v-sync checkbox (replaced with framerate setting)
- added 'normal' argument to on_use_on_block
- increased debug-panel width

## Fixes

- the randomness factor of the generation seed when creating a world
- behavior of blocks with a pipe rotation profile in combination with the grounded property
- behavior of the u_timer uniform variable when paused
- torch material
- Lua stack leaks
- behavior of Lua functions in coroutines
- support for wav sounds
- [issue #239](https://github.com/MihailRis/VoxelEngine-Cpp/issues/239)
- errors when rebooting the world
- incorrect lighting of AABB blocks
- camera height limitation
- incorrect timing of the mouseRelease event
- 'gravity' does not work on hud overlays
- overlay 'on_close' not called on window close
- delta time plotter is interactive
