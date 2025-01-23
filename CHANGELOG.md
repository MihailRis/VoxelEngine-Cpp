# 0.26 - 2025.01.24

[Documentation](https://github.com/MihailRis/VoxelEngine-Cpp/tree/release-0.26/doc/en/main-page.md) for 0.26

Table of contents:

- [Added](#added)
    - [Changes](#changes)
    - [Functions](#functions)
- [Fixes](#fixes)

## Added

- headless mode `--headless`
- script execution mode `--headless --script filename`
- test execution mode `--headless --test filename`
- vctest console application
- libraries:
    - app
    - byteutil
- in-game chat
- text markup: Markdown
- syntax-highlighting: Lua
- http post requests
- `Scripts` menu page for app scripts
- binding `hud.chat`
- user-defined console.submit
- events:
    - on_chunk_present
    - on_chunk_remove
    - on_inventory_open
    - on_inventory_closed
- [canvas](https://github.com/MihailRis/VoxelEngine-Cpp/pull/444) ui node
- settings:
    - `graphics.dense-render`
- block properties:
    - `culling`
- particles properties:
    - `angle_spread`
    - `min_angular_vel`, `max_angular_vel`
- bytearray support in serializers
- ui properties:
    - uinode: `cursor`
    - textbox: `markup`, `syntax`, `text-color`
    - label: `markup`
- base pack:
    - add transparent leaves render mode
    - add falling leaves particles
    - 'states' parameter in base:falling_block
- nameless worlds
- SIGTERM handler
- project:
    - clang Windows workflow
    - engine tests

### Changes

- moved `devtools.console` binding handler to Lua
- move `key:escape` binding handler to Lua
- upgrade dead emitters garbage collection
- reserved player entity ids: `0` - none (example: dead), `-1` - auto (spawns new one)
- input.add_callback("key:name") support and add optional `owner` argument

### Functions

- app.tick
- app.sleep
- app.sleep_until
- app.new_world
- app.open_world
- app.save_world
- app.close_world
- app.reopen_world
- app.delete_world
- app.config_packs
- app.reconfig_packs
- app.get_setting
- app.set_setting
- app.get_version
- app.get_setting_info
- app.load_content
- app.reset_content
- app.is_content_loaded
- app.quit
- entity:get_player
- start_coroutine
- gui.clear_markup
- gui.escape_markup
- gui.alert
- gui.confirm
- gui.load_document
- console.get
- world.get_chunk_data
- world.set_chunk_data
- world.save_chunk_data
- world.count_chunks
- player.create
- player.delete
- player.is_suspended
- player.set_suspended
- player.is_loaded_chunks
- player.set_loading_chunks
- network.post
- table.shuffle
- table.deep_copy
- math.normalize
- math.round
- byteutil.pack
- byteutil.unpack
- file.name
- file.stem
- file.ext
- file.prefix
- hud.set_allow_pause
Methods:
- uinode:reposition
- socket:available
- textbox:lineAt
- textbox:linePos
New overloads:
- block.get_X, block.get_Y, block.get_Z
- player.get_rot
- Bytearray:append

## Fixes
- [fix on_block_interact & fix segfault after engine finished](https://github.com/MihailRis/VoxelEngine-Cpp/commit/d1f92c21d0bbdf2df0eb3b31c5637bdf7110444c)
- [fix item.properties](https://github.com/MihailRis/VoxelEngine-Cpp/commit/92fb19ba5e2307fdbcbf5d0e55f9c0712be45f72)
- [fix base:bazalt durability](https://github.com/MihailRis/VoxelEngine-Cpp/commit/a036c5e383135dc0f9b086e244188d1ceb3f0bf2)
- [fix camera-related bugs](https://github.com/MihailRis/VoxelEngine-Cpp/commit/0d071ab0141edbf087f3ec03505792740023c01e)
- [fix: grabbed item is deleted on inventory close](https://github.com/MihailRis/VoxelEngine-Cpp/commit/2787f2fc5495004f6029644ed5221f3abfc0c68f)
- [fix block overriding](https://github.com/MihailRis/VoxelEngine-Cpp/commit/cda34e3975a42696ea31a1b0018731e746cd13bb)
- [fix faces culling when 'light-passing' is false](https://github.com/MihailRis/VoxelEngine-Cpp/commit/954724c8378da525fc7349c018e9351c5bdfdf8f)
- [fix particles lighting](https://github.com/MihailRis/VoxelEngine-Cpp/commit/6be640458d6b4ae46866b342ca0f26e561ead125)
- [fix non-skipping particles](https://github.com/MihailRis/VoxelEngine-Cpp/pull/421/commits/f1c7317c5ab2a148e5188e091cd1aa3490dc8b4d)
- [fix content stats](https://github.com/MihailRis/VoxelEngine-Cpp/commit/97eef3ef1900157a9648bade8e06b203b99ee6f6)
- [fix byte manipulation functions](https://github.com/MihailRis/VoxelEngine-Cpp/commit/9490d1f7eacb00f56112dfdd1ea12bb9c3ca528d)
- [fix error handling in events and runnables](https://github.com/MihailRis/VoxelEngine-Cpp/commit/03a3062940ebfc4e8f0b3efc5930c71f8d07b604)
- [fix small dumb legacy memory leak](https://github.com/MihailRis/VoxelEngine-Cpp/commit/4d0b9f049b79322959e4aefd95eedc665e87d087)
- [fix grass lighting](https://github.com/MihailRis/VoxelEngine-Cpp/commit/9d7816a286fb3a7269b5220502354720e4d2726b)
- [small fixes in translation.](https://github.com/MihailRis/VoxelEngine-Cpp/commit/d25452784d68be19821dc917ad15bc0a92d81bd9)
- [fix errors handling in event handlers](https://github.com/MihailRis/VoxelEngine-Cpp/commit/f62fc5a039dca70219fb2b38f61fc53a2542adf7)
- [fix lua stack manipulations](https://github.com/MihailRis/VoxelEngine-Cpp/commit/e7555448cf0df86995b40d67fa58de1ca78f8105)
- [fix lua::create_lambda](https://github.com/MihailRis/VoxelEngine-Cpp/commit/40cdebb175014736e35bc31ecc93ae72fb00a6e9)
- [fix some UB](https://github.com/MihailRis/VoxelEngine-Cpp/commit/b5999fe36420d116674abc353ed3dad739ac5f70)
- [fix rigidbody:is_enabled](https://github.com/MihailRis/VoxelEngine-Cpp/commit/2adfbdb19226b2685848131073a56b354706433d)
- [fix panel elements removal](https://github.com/MihailRis/VoxelEngine-Cpp/commit/c6951e09651149463528bdffbc2cba4ea41de4a4)
- [fix infinite block fields conversion requests](https://github.com/MihailRis/VoxelEngine-Cpp/commit/0494db91872abff500cfc153a32035ee3f2745ae)
- [fix data_buffer:put_number](https://github.com/MihailRis/VoxelEngine-Cpp/commit/e247902cc6ffdaa6beab391fcfdaea7f021ab063)
- [fix textbox horizontal scroll & fix console log width](https://github.com/MihailRis/VoxelEngine-Cpp/commit/13fde2116d095b9393c4f5804ba23071e5f56ad6)
- [fix is_array](https://github.com/MihailRis/VoxelEngine-Cpp/pull/420)
- [fix neighbour chunk update](https://github.com/MihailRis/VoxelEngine-Cpp/pull/404)
- [fix lamp material](https://github.com/MihailRis/VoxelEngine-Cpp/commit/57356e1d64d6d9d7e8d59b078543b290e998ad00)
