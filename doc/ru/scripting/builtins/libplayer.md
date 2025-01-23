# Библиотека *player*

```lua
player.create(name: str) -> int
```

Создаёт игрока и возвращает его id.

```lua
player.delete(id: int)
```

Удаляет игрока по id.

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
player.get_rot(playerid: int, interpolated: bool=false) -> number, number, number
```

Возвращает x, y, z вращения камеры (в радианах). Интерполяция актуальна в случаях, когда частота обновления вращения ниже частоты кадров.

```lua
player.set_rot(playerid: int, x: number, y: number, z: number)
```

Устанавливает x, y вращения камеры (в радианах)

```lua
player.get_inventory(playerid: int) -> int, int
```

Возвращает id инвентаря игрока и индекс выбранного слота (от 0 до 9)

```lua
player.is_flight(playerid: int) -> bool
player.set_flight(playerid: int, bool)
```

Геттер и сеттер режима полета

```lua
player.is_noclip(playerid: int) -> bool
player.set_noclip(playerid: int, bool)
```

Геттер и сеттер noclip режима (выключенная коллизия игрока)

```lua
player.is_infinite_items(playerid: int) -> bool
player.set_infinite_items(playerid: int, bool)
```

Геттер и сеттер бесконечных предметов (не удаляются из инвентаря при использовании)

```lua
player.is_instant_destruction(playerid: int) -> bool
player.set_instant_destruction(playerid: int, bool)
```

Геттер и сеттер мнгновенного разрушения блоков при активации привязки `player.destroy`.

```lua
player.is_loading_chunks(playerid: int) -> bool
player.set_loading_chunks(playerid: int, bool)
```

Геттер и сеттер свойства, определяющего, прогружает ли игрок чанки вокруг.

```lua
player.set_spawnpoint(playerid: int, x: number, y: number, z: number) 
player.get_spawnpoint(playerid: int) -> number, number, number
```

Сеттер и геттер точки спавна игрока

```lua
player.set_name(playerid: int, name: str) 
player.get_name(playerid: int) -> str
```

Сеттер и геттер имени игрока

```lua
player.set_selected_slot(playerid: int, slotid: int)
```

Устанавливает индекс выбранного слота

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
