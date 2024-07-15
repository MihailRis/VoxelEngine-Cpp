# Scripting

Project uses LuaJIT as a scripting language.

Subsections:
- [Engine events](scripting/events.md)
- [User input](scripting/user-input.md)
- [Filesystem and serialization](scripting/filesystem.md)
- [UI properties and methods](scripting/ui.md)
- [Entities and components](scripting/ecs.md)
- [Libraries](#)
    - [block](scripting/builtins/libblock.md)
    - [entities](scripting/builtins/libentities.md)
    - [mat4](scripting/builtins/libmat4.md)
- [Module core:bit_converter](scripting/modules/core_bit_converter.md)
- [Module core:data_buffer](scripting/modules/core_data_buffer.md)
- [Module core:vector2, core:vector3](scripting/modules/core_vector2_vector3.md)


## Core functions

```lua
require "packid:module_name" -- load Lua module from pack-folder/modules/
-- no extension included, just name
```

## *pack* library

```python
pack.is_installed(packid: str) -> bool
```

Check if specified pack is installed in the world

```python
pack.data_file(packid: str, filename: str) -> str
```

Returns data file path like `world:data/packid/filename` 
and creates missing directories.

Use this function when saving pack settings or other data to the world.

Example:
```lua
file.write(pack.data_file(PACK_ID, "example.txt"), text)
```

For pack *containermod* will write text to the file `world:data/containermod/example.txt`

## *player* library

```python
player.get_pos(playerid: int) -> number, number, number
```

Returns x, y, z coordinates of the player

```python
player.set_pos(playerid: int, x: number, y: number, z: number)
```

Set player position

```python
player.get_rot(playerid: int) -> number, number, number
```

Returns x, y, z of camera rotation (radians)

```python
player.set_rot(playerid: int, x: number, y: number, z: number)
```

Set camera rotation (radians)

```python
player.get_inventory(playerid: int) -> int, int
```

Returns player inventory ID and selected slot index (0-9)

```python
player.is_flight() -> bool
player.set_flight(bool)
```

Getter and setter for player flight mode

```python
player.is_noclip() -> bool
player.set_noclip(bool)
```

Getter and setter for player noclip mode (collisions disabled)

``` python
player.set_spawnpoint(playerid: int, x: number, y: number, z: number)
player.get_spawnpoint(playerid: int) -> number, number, number
```

Point setter and getter added by player

```python
player.get_selected_block(playerid: int) -> x,y,z
```

Returns position of the selected block or nil

```python
player.get_selected_entity(playerid: int) -> int
```

Returns unique indentifier of the entity selected by player

```python
player.get_entity(playerid: int) -> int
```

Returns unique identifier of the player entity

## *world* library

## Библиотека *world*

```python
world.get_list() -> tables array {
	name: str,
	icon: str
}
```

Retuns worlds information: name and preview/icon (loading automatically).

```python
world.get_day_time() -> number
```

Returns current day time in range \[0.0-1.0\] where 0.0 and 1.0 - midnight, 0.5 - noon.

```python
world.set_day_time(time: number)
```

Set day time value.

```python
world.set_day_time_speed(value: number)
```

Sets the specified speed for game time.

```python
world.get_day_time_speed() -> number
```

Returns the speed for game time.

```python
world.get_total_time() -> number
```

Returns total time passed in the world

```python
world.get_seed() -> int
```

Returns world seed.

``` python
world.is_day() -> boolean
```

Proves that this is the current time during the day. From 0.2(8 am) to 0.8(8 pm)

``` python
world.is_night() -> bool
```

Checks that it is the current time at night. From 0.8(8 pm) to 0.2(8 am)

```python
world.exists() -> bool
```

Checks the existence of a world by name.

## *pack* library

```python
pack.get_folder(packid: str) -> str
```

Returns installed content-pack folder.

```python
pack.is_installed(packid: str) -> bool
```

Check if the world has specified pack installed.

```python
pack.get_installed() -> strings array
```

Returns all installed content-pack ids.

```python
pack.get_available() -> strings array
```

Returns the ids of all content packs available but not installed in the world.

```python
pack.get_base_packs() -> strings array
```

Returns the id of all base packages (non-removeable)

```python
pack.get_info(packid: str) -> {
  id: str,
  title: str,
  creator: str,
  description: str,
  version: str,
  icon: str,
  dependencies: optional strings array
}
```

Returns information about the pack (not necessarily installed).
- icon - name of the preview texture (loading automatically)
- dependencies - strings following format `{lvl}{id}`, where lvl:
  - `!` - required
  - `?` - optional
  - `~` - weak
  for example `!teal`

## *gui* library

The library contains functions for accessing the properties of UI elements. Instead of gui, you should use an object wrapper that provides access to properties through the __index, __newindex meta methods:

Example:

```lua
print(document.some_button.text) -- where 'some_button' is an element id
document.some_button.text = "new text"
```

```python
gui.str(text: str, context: str) -> str
```

Returns translated text.

```python
gui.get_viewport() -> {int, int}
```

Returns size of the main container (window).

```python
gui.get_env(document: str) -> table
```

Returns environment (global variables table) of the specified document.

```python
get_locales_info() -> table of tables where
 key - locale id following isolangcode_ISOCOUNTRYCODE format
 value - table {
  name: str # name of the locale in its language
 }
```

Returns information about all loaded locales (res/texts/\*).

## *inventory* library

Library for inventories interaction.

```python
inventory.get(invid: int, slot: int) -> int, int
```

Requires an inventory ID and slot index. Returns item ID and count. ID = 0 (core:empty) means that slot is empty.

```python
inventory.set(invid: int, slot: int, itemid: int, count: int)
```

Set slot content.

```python
inventory.size(invid: int) -> int
```

Returns inventory size (slots number). Throws an exception if there's no inventory having specified ID.

```python
inventory.add(invid: int, itemid: int, count: int) -> int
```

Add an item to the specified inventory. Returns remaining count if could not to add fully.

```python
inventory.get_block(x: int, y: int, z: int) -> int
```

Returns block inventory ID or 0.

```python
inventory.bind_block(invid: int, x: int, y: int, z: int)
```

Bind inventory to the specified block.

```python
inventory.unbind_block(x: int, y: int, z: int)
```

Unbind inventory from the specified block.

> [!WARNING]
> Unbound inventories will be deleted on world close.

```python
inventory.clone(invid: int) -> int
```

Create inventory copy. Returns the created copy ID.

```python
inventory.move(invA: int, slotA: int, invB: int, slotB: int)
```

Move item from slotA of invA to slotB of invB. invA may be the same as invB.
If slotB will be chosen automaticly if argument is not specified.

## *item* library

```python
item.name(itemid: int) -> str
```

Returns item string ID (name) by index

```python
item.index(name: str) -> int
```

Returns item integer ID (index) by name

```python
item.stack_size(itemid: int) -> int
```

Returns max stack size for the item

```python
item.defs_count() -> int
```

Returns count of available item IDs.

```python
item.icon(itemid: int) -> str
```

Returns item icon name to use in 'src' property of an image element

## *hud* library


```python
hud.open_inventory()
```

Open player inventory

```python
hud.close_inventory()
```

Close inventory

```python
hud.open_block(x: int, y: int, z: int) -> int, str
```

Open block UI and inventory. Throws an exception if block has no UI layout.

Returns block inventory ID (if *"inventory-size"=0* a virtual inventory will be created), and UI layout ID.

```python
hud.show_overlay(layoutid: str, playerinv: bool)
```

Show overlay with layout specified. Shows player inventory also if playerinv is **true**

> [!NOTE]
> Only one block may be open at same time

```python
hud.open_permanent(layoutid: str)
```

Add element to the screen. The element will be removed on world close only.
**inventory** element will be bound to the player inventory.

```python
hud.close(layoutid: str)
```

Remove an element from the screen


```python
hud.get_block_inventory() -> int
```

Get open block inventory ID or 0.

```python
hud.get_player() -> int
```

Gives the ID of the player that the UI is bound to.

```python
hud.pause()
```

Opens the pause menu

```python
hud.resume()
```

Closes the pause menu.

```python
hud.is_paused() -> bool
```

Returns true if pause menu is open.

```python
hud.is_inventory_open() -> bool
```

Returns true if inventory is open or overlay is shown.

### *time* library

```python
time.uptime() -> float
```

Returns time elapsed since the engine started.

```python
time.delta() -> float
```

Returns time elapsed since the last frame.
