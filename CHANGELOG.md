# 0.27 - 2025.03.??

[Documentation](https://github.com/MihailRis/VoxelEngine-Cpp/tree/release-0.27/doc/en/main-page.md) for 0.27

Table of contents:

- [Added](#added)
    - [Changes](#changes)
    - [Functions](#functions)
- [Fixes](#fixes)

## Added

- items data
- weather
- work with ZIP-files
- splitbox ui element
- history in textbox
- reloading content scripts within the editor
- libraries:
	- gfx.weather
- [add side mouse buttons](https://github.com/MihailRis/VoxelEngine-Cpp/pull/461)
- item properties:
	- uses
	- uses-display
- textbox attributes:
	- oncontrolkey
- multiline tooltips suppoirt
- ['@' prefix support in tooltips](https://github.com/MihailRis/VoxelEngine-Cpp/commit/fa990e393986aad07e9fb9a570338b08fd4f9a0c)

### Changes

- file subsystem upgrade
- code editor upgrade
- block materials update
- actual file path now may be specified in audio.play_stream/play_stream_2d call.

### Functions

- file.create_zip
- file.mount
- file.unmount
- file.path
- file.parent
- file.join
- pack.request_writeable
- crc32
- table.merge
- table.map
- table.filter
- table.set_default
- table.flat
- table.deep_flat
- table.sub
- math.sum
- string.pad
- string.left_pad
- string.right_pad
- block.reload_script
- world.reload_script
- entities.reload_component
- hud.reload_script
- item.reload_script
- gfx.weather.change
- gfx.weather.get_current
- gfx.weather.get_current_data
- gfx.weather.get_fall_intensity
- gfx.weather.is_transition
- inventory.decrement
- inventory.use
- inventory.set_count
- inventory.get_data
- inventory.set_data
- inventory.get_all_data
- inventory.has_data
- item.uses
- core.capture_output

Methods:

- canvas:set_data
- canvas:line
- canvas:blit
- canvas:clear
- canvas:create_texture

## Fixes

- [fix incorrect entity transform scale & rotation combination](https://github.com/MihailRis/VoxelEngine-Cpp/commit/b832b9850f709053b43714bb0e3481560711e872)
- [fix syntax highlighting not refreshing on selected text removal](https://github.com/MihailRis/VoxelEngine-Cpp/commit/33795f06f710de569a24b2d1937310b16dd040f5)
- [fix TextBox behavior with markup used](https://github.com/MihailRis/VoxelEngine-Cpp/commit/31d5cb68800d965a668a67fd60bb4d3365181242)
- [fix: console/chat open still available when menu is open & cleanup](https://github.com/MihailRis/VoxelEngine-Cpp/commit/c7061c70cb3b6382197c93d9442bfbb4c9fc1def)
- [fix: Label.cpp: uinode size](https://github.com/MihailRis/VoxelEngine-Cpp/commit/6e8a4bacff77b7eba39393bcc8a5fb417a288884)
- [fix: camera rotation not updating when menu is open](https://github.com/MihailRis/VoxelEngine-Cpp/commit/801354c15d96343b00963183f6f08219bd375ee8)
- [fix stack split (right click) with item data](https://github.com/MihailRis/VoxelEngine-Cpp/commit/6ef82982bdc813bfc5c0ed2437a1a3fa9a2242d1)
- [fix stack increment (right click) with item data](https://github.com/MihailRis/VoxelEngine-Cpp/commit/449b0ebca464f913f63913ad7af91811c717c349)
- [fix caves generation](https://github.com/MihailRis/VoxelEngine-Cpp/commit/4c8b0824181f05aa5611a1e077f999788f575c42)
- [fix: incorrect UV on custom model blocks](https://github.com/MihailRis/VoxelEngine-Cpp/commit/12105c29335481de429c8e6acc953e8cbcdc6ecf)
- [bug fix content_menu.xml.lua](https://github.com/MihailRis/VoxelEngine-Cpp/commit/15e143afb0364f816a1033868b2501c9a9e69fda)
- [fix: disabled bodies not skipping in entities raycast](https://github.com/MihailRis/VoxelEngine-Cpp/commit/5e10afc9b5d6582617fe34ec37abdb682e46197a)
- [table.filter bug fix](https://github.com/MihailRis/VoxelEngine-Cpp/pull/481)
- [fix: incorrect canvas Y direction](https://github.com/MihailRis/VoxelEngine-Cpp/commit/ee3fbc68314865384d2a7d78a22221b5987979a4)
- [fix fatal error on audio.play_sound in headless-mode](https://github.com/MihailRis/VoxelEngine-Cpp/commit/a74a4fcf5366c8f78625f7087f42a8150f6896e0)
- [fix BasicParser '\v' and '\x' escapes](https://github.com/MihailRis/VoxelEngine-Cpp/commit/8b0935d2b745acb11cba9c8bc647b4186d1817a3)
- [fix: entity:despawn in on_save not preventing entity save](https://github.com/MihailRis/VoxelEngine-Cpp/pull/480)
- [fix syntax highlighting unicode support](https://github.com/MihailRis/VoxelEngine-Cpp/pull/475)
- [fix reading binary file as string](https://github.com/MihailRis/VoxelEngine-Cpp/commit/104cc116456aba6e569892521ecae92d8bf6b9ff)
- [fix inventory.move_range](https://github.com/MihailRis/VoxelEngine-Cpp/commit/f5a4b5ca79806418b0736d3f84e9efa8d9992a34)
