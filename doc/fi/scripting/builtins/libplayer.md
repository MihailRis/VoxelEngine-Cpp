# Kirjasto *player*

```lua
player.get_pos(playerid: int) -> number, number, number
```

Palauttaa soittimen x, y, z koordinaatit

```lua
player.set_pos(playerid: int, x: number, y: number, z: number)
```

Asettaa soittimen x, y, z koordinaatit

```lua
player.get_vel(playerid: int) -> number, number, number
```

Palauttaa soittimen lineaarisen nopeuden x, y, z

```lua
player.set_vel(playerid: int, x: number, y: number, z: number)
```

Asettaa soittimen lineaarisen nopeuden x, y, z

```lua
player.get_rot(playerid: int) -> number, number, number
```

Palauttaa kameran kierron x, y, z (radiaaneina)

```lua
player.set_rot(playerid: int, x: number, y: number, z: number)
```

Asettaa kameran x, y kierron (radiaaneina)

```lua
player.get_inventory(playerid: int) -> int, int
```

Palauttaa pelaajan varastotunnuksen ja valitun paikan indeksin (0-9)

```lua
player.is_flight() -> bool
player.set_flight(bool)
```

Lentotilan saaja ja asettaja

```lua
player.is_noclip() -> bool
player.set_noclip(bool)
```

noclip-tilan saanti ja asettaja (soittimen törmäys pois käytöstä)

```lua
player.set_spawnpoint(playerid: int, x: number, y: number, z: number) 
player.get_spawnpoint(playerid: int) -> number, number, number
```

Player spawn point setter ja getter

```lua
player.get_selected_block(playerid: int) -> x,y,z
```

Palauttaa valitun lohkon koordinaatit tai nollan

```lua
player.get_selected_entity(playerid: int) -> int
```

Palauttaa soittimen kohteena olevan kokonaisuuden yksilöllisen tunnisteen

```lua
player.get_entity(playerid: int) -> int
```

Palauttaa pelaajakokonaisuuden yksilöllisen tunnisteen
