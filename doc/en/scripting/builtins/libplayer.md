# *player* library

```lua
player.create(name: str) -> int
```

Creates a player and returns id.

```lua
player.delete(id: int)
```

Deletes a player by id.

```lua
player.get_pos(playerid: int) -> number, number, number
```

Returns x, y, z coordinates of the player

```lua
player.set_pos(playerid: int, x: number, y: number, z: number)
```

Set player position

``` lua
player.get_vel(playerid: int) -> number, number, number
```

Returns the x, y, z linear velocity of the player

``` lua
player.set_vel(playerid: int, x: number, y: number, z: number)
```

Sets x, y, z player linear velocity

```lua
player.get_rot(playerid: int, interpolated: bool) -> number, number, number
```

Returns x, y, z of camera rotation (radians). Interpolation is relevant in cases where the rotation refresh rate is lower than the frame rate.

```lua
player.set_rot(playerid: int, x: number, y: number, z: number)
```

Set camera rotation (radians)

```lua
player.get_inventory(playerid: int) -> int, int
```

Returns player inventory ID and selected slot index (0-9)

```lua
player.is_flight(playerid: int) -> bool
player.set_flight(playerid: int, bool)
```

Getter and setter for player flight mode

```lua
player.is_noclip(playerid: int) -> bool
player.set_noclip(playerid: int, bool)
```

Getter and setter for player noclip mode (collisions disabled)

```lua
player.is_infinite_items(playerid: int) -> bool
player.set_infinite_items(playerid: int, bool)
```

Getter and setter for infinite items (not removed from inventory after use)

```lua
player.is_instant_destruction(playerid: int) -> bool
player.set_instant_destruction(playerid: int, bool)
```

Getter and setter for instant destruction of blocks when the `player.destroy` binding is activated.

```lua
player.is_loading_chunks(playerid: int) -> bool
player.set_loading_chunks(playerid: int, bool)
```

Getter and setter of the property that determines whether the player is loading chunks.

``` lua
player.set_spawnpoint(playerid: int, x: number, y: number, z: number)
player.get_spawnpoint(playerid: int) -> number, number, number
```

Spawn point setter and getter

```lua
player.is_suspended(pid: int) -> bool
player.set_suspended(pid: int, suspended: bool)
```

Setter and getter for the player's suspended status.

When suspended, the entity is deleted and the player is disabled from the world simulation.

```lua
player.set_name(playerid: int, name: str)
player.get_name(playerid: int) -> str
```

Player name setter and getter

```lua
player.set_selected_slot(playerid: int, slotid: int)
```

Sets the selected slot index

```lua
player.get_selected_block(playerid: int) -> x,y,z
```

Returns position of the selected block or nil

```lua
player.get_selected_entity(playerid: int) -> int
```

Returns unique indentifier of the entity selected by player

```lua
player.get_entity(playerid: int) -> int
```

Returns unique identifier of the player entity
