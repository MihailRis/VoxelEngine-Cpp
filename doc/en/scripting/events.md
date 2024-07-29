# Engine events

## Block events

Callbacks specified in block script.

```lua
function on_placed(x, y, z, playerid)
```

Called on block placed by player

```lua
function on_broken(x, y, z, playerid)
```

Called on block broken by player

```lua
function on_interact(x, y, z, playerid) -> bool
```

Called on block RMB click interaction. Prevents block placing if **true** returned.

```lua
function on_update(x, y, z)
```

Called on block update (near block changed)

```lua
function on_random_update(x, y, z)
```

Called on random block update (grass growth)

```lua
function on_blocks_tick(tps: int)
```

Called tps (20) times per second.

## Item events

Callbacks specified in item script.

```lua
function on_use(playerid: int)
```

Called on RMB click out of a block.

```lua
function on_use_on_block(x: int, y: int, z: int, playerid: int, normal: vec3)
```

Called on block RMB click. Prevents block **placing-block** placing if returns **true**

```lua
function on_block_break_by(x: int, y: int, z: int, playerid: int)
```

Called on block LMB click (unbreakable blocks included).  Prevents block destruction if returns **true**.

## World events

Callbacks specified in *world.lua* script.

```lua
function on_world_open()
```

Called on world open.

```lua
function on_world_save()
```

Called before world save.

```lua
function on_world_tick()
```

Called 20 times per second

```lua
function on_world_quit()
```

Called on world close (after saving)

## Layout events

Script *layouts/layout_name.xml.lua* events.

```lua
function on_open(invid: int, x: int, y: int, z: int)
```

Called on element added to the screen.
invid=0 if no inventory bound
x,y,z=0 if no block bound

```lua
function on_close(invid: int)
```

Called on element removed from the screen.

## HUD events

Callbacks specified in *hud.lua* script.


```lua
function on_hud_open(playerid: int)
```

Called after world open.

```lua
function on_hud_close(playerid: int)
```

Called on world close (before saving)
