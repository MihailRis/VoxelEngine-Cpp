# Библиотека *player*

```lua
player.get_pos(playerid: int) -> number, number, number
```

Возвращает x, y, z координаты игрока

```lua
player.set_pos(playerid: int, x: number, y: number, z: number)
```

Устанавливает x, y, z координаты игрока

```lua
player.get_vel(playerid: int) -> number, number, number
```

Возвращает x, y, z линейной скорости игрока

```lua
player.set_vel(playerid: int, x: number, y: number, z: number)
```

Устанавливает x, y, z линейной скорости игрока

```lua
player.get_rot(playerid: int) -> number, number, number
```

Возвращает x, y, z вращения камеры (в радианах)

```lua
player.set_rot(playerid: int, x: number, y: number, z: number)
```

Устанавливает x, y вращения камеры (в радианах)

```lua
player.get_inventory(playerid: int) -> int, int
```

Возвращает id инвентаря игрока и индекс выбранного слота (от 0 до 9)

```lua
player.is_flight() -> bool
player.set_flight(bool)
```

Геттер и сеттер режима полета

```lua
player.is_noclip() -> bool
player.set_noclip(bool)
```

Геттер и сеттер noclip режима (выключенная коллизия игрока)

```lua
player.set_spawnpoint(playerid: int, x: number, y: number, z: number) 
player.get_spawnpoint(playerid: int) -> number, number, number
```

Сеттер и геттер точки спавна игрока

```lua
player.get_selected_block(playerid: int) -> x,y,z
```

Возвращает координаты выделенного блока, либо nil

```lua
player.get_selected_entity(playerid: int) -> int
```

Возвращает уникальный идентификатор сущности, на которую нацелен игрок

```lua
player.get_entity(playerid: int) -> int
```

Возвращает уникальный идентификатор сущности игрока
