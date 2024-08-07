# *player* library

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
player.get_rot(playerid: int) -> number, number, number
```

Returns x, y, z of camera rotation (radians)

```lua
player.set_rot(playerid: int, x: number, y: number, z: number)
```

Set camera rotation (radians)

```lua
player.get_inventory(playerid: int) -> int, int
```

Returns player inventory ID and selected slot index (0-9)

```lua
player.is_flight() -> bool
player.set_flight(bool)
```

Getter and setter for player flight mode

```lua
player.is_noclip() -> bool
player.set_noclip(bool)
```

Getter and setter for player noclip mode (collisions disabled)

``` lua
player.set_spawnpoint(playerid: int, x: number, y: number, z: number)
player.get_spawnpoint(playerid: int) -> number, number, number
```

Point setter and getter added by player

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
