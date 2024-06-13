# Scripting

Project uses LuaJIT as a scripting language.

Subsections:
- [Engine events](scripting/events.md)
- [User input](scripting/user-input.md)
- [Filesystem and serialization](scripting/filesystem.md)
- [Module core:bit_converter](scripting/modules/core_bit_converter.md)
- [Module core:data_buffer](scripting/modules/core_data_buffer.md)
- [Module core:Vector2, core:Vector3](scripting/modules/core_Vector2&&Vector3.md)


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

## *world* library

```python
world.get_day_time() -> number
```

Returns current day time in range \[0.0-1.0\] where 0.0 and 1.0 - midnight, 0.5 - noon.

```python
world.set_day_time(time: number)
```

Set day time value.

```python
world.get_total_time() -> number
```

Returns total time passed in the world

```python
world.get_seed() -> int
```

Returns world seed.

## *pack* library

```python
pack.get_folder(packid: str) -> str
```

Returns installed content-pack folder

```python
pack.is_installed(packid: str) -> bool
```

Check if the world has specified pack installed

```python
pack.get_installed() -> array of strings
```

Returns all installed content-pack ids

## *gui* library

Library contains ui elements access functions. Library should not be directly used, because script *layouts/layout_name.xml.lua* already has a generated variable **document** (instance of **Document**)

Example:

```lua
print(document.some_button.text) -- where 'some_button' is an element id
document.some_button.text = "new text"
```

## **inventory** library

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

## *block* library

```python
block.name(blockid: int) -> str
```

Returns block string ID (name) by index

```python
block.index(name: str) -> int
```

Returns block integer ID (index) by name

```python
block.get(x: int, y: int, z: int) -> int
```

Returns integer ID by block position

```python
block.get_states(x: int, y: int, z: int) -> int
```

Returns block state (rotation + additional information) as an integer.

```python
block.set(x: int, y: int, z: int, id: int, states: int)
```

Set block with specified integer ID and state (default - 0) at specified position.

> [!WARNING]
> `block.set` does not trigger on_placed.

```python
block.is_solid_at(x: int, y: int, z: int) -> bool
```

Check if block at the specified position is solid.

```python
block.is_replaceable_at(x: int, y: int, z: int) -> bool
```
Check if block may be placed at specified position. (Examples: air, water, grass, flower)

```python
block.defs_count() -> int
```

Returns count of available block IDs.

Following three functions return direction vectors based on block rotation.


```python
block.get_X(x: int, y: int, z: int) -> int, int, int
```

Returns X: integer direction vector of the block at specified coordinates.
Example: no rotation: 1, 0, 0

```python
block.get_Y(x: int, y: int, z: int) -> int, int, int
```

Returns Y: integer direction vector of the block at specified coordinates.
Example: no rotation: 0, 1, 0

```python
block.get_Z(x: int, y: int, z: int) -> int, int, int
```

Returns Z: integer direction vector of the block at specified coordinates.
Example: no rotation: 0, 0, 1


```python
block.get_rotation(x: int, y: int, z: int) -> int
```

Returns block rotation index based on used profile.

```python
block.set_rotation(x: int, y: int, z: int, rotation: int)
```

Set block rotation by index.

### User bits

Part of a voxel data used for scripting. Size: 8 bit.

```python
block.get_user_bits(x: int, y: int, z: int, offset: int, bits: int) -> int
``` 

Get specified bits as an unsigned integer.

```python
block.set_user_bits(x: int, y: int, z: int, offset: int, bits: int, value: int) -> int
```
Set specified bits.

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

Get open block inventory ID or 0

## Engine libraries

### file

Filesystem interaction library.

```python
file.resolve(path: str) -> str
```

Function turns *entry_point:path* (example *user:worlds/house1*) to a regular path. (example *C://Users/user/.voxeng/worlds/house1*)

> [!NOTE]
> The function should be used for debug only. *entry_point:path* notation is required in all **file** functions.

Resulting path is not canonical and may be relative.

```python
file.read(path: str) -> str
```

Read whole text file.

```python
file.read_bytes(path: str) -> array of integers
```

Read file into bytes array.

```python
file.write(path: str, text: str) -> nil
```

Overwrite text file.

```python
file.write_bytes(path: str, data: array of integers)
```

Overwrite binary file with bytes array.

```python
file.length(path: str) -> int
```

Get file length (bytes) or 0.

```python
file.exists(path: str) -> bool
```

Check if file or directory exist.

```python
file.isfile(path: str) -> bool
```

Check if the path points to a file.

```python
file.isdir(path: str) -> bool
```

Check if the path points to a directory.

```python
file.mkdir(path: str) -> bool
```

Create directory. Returns true if new directory created

```python
file.mkdirs(path: str) -> bool
```

Create directories chain. Returns true if new directory created

### time

```python
time.uptime() -> float
```

Returns time elapsed since the engine started.

## Available modules

### TOML serialization/deserialization

```lua
local toml = require "core:toml"

local t = {a=53, b=42, s="test", sub={x=1, y=6}}
local s = toml.serialize(t)
print(s)
local t2 = toml.deserialize(s) 
```
output:
```toml
b = 42
s = "test"
a = 53
[sub]
y = 6
x = 1
```
