# *app* library

A library for high-level engine control, available only in script/test mode.

The script/test name without path and extension is available as `app.script`. The file path can be obtained as:
```lua
local filename = "script:"..app.script..".lua"
```

## Functions

```lua
app.tick()
```

Executes one tick of the engine main loop.

```lua
app.sleep(time: number)
```

Waits for the specified time in seconds, executing the engine main loop.

```lua
app.sleep_until(
    -- function that checks the wait finishing condition
    predicate: function() -> bool,
    -- maximum number of engine cycle ticks, after which
    -- an exception "max ticks exceed" will be thrown
    [optional] max_ticks = 1e9
)
```

Waits for the condition checked by the function to be true, executing the engine main loop.

```lua
app.quit()
```

Quits the engine, printing the call stack to track where the function was called.

```lua
app.reconfig_packs(
    -- packs to add
    add_packs: table,
    -- packs to remove
    remove_packs: table
)
```

Updates the packs configuration, checking its correctness (dependencies and packs existence).

To remove all packs from the configuration, you can use `pack.get_installed()`:

```lua
app.reconfig_packs({}, pack.get_installed())
```

In this case, `base` will also be removed from the configuration.

```lua
app.new_world(
    -- world name
    name: str,
    -- seed of generation
    seed: str,
    -- name of generator
    generator: str
)
```

Creates a new world and opens it.

```lua
app.open_world(name: str)
```

Opens a world by name.

```lua
app.reopen_world()
```

Reopens the world.

```lua
app.close_world(
    -- save the world before closing
    [optional] save_world: bool=false
)
```

Closes the world.

```lua
app.delete_world(name: str)
```

Deletes a world by name.

```lua
app.get_version() -> int, int
```

Returns the major and minor engine versions.

```lua
app.get_setting(name: str) -> value
```

Returns the value of a setting. Throws an exception if the setting does not exist.

```lua
app.set_setting(name: str, value: value)
```

Sets the value of a setting. Throws an exception if the setting does not exist.

```lua
app.get_setting_info(name: str) -> {
    -- default value
    def: value
    -- minimum value
    [numeric settings only] min: number,
    -- maximum value
    [numeric settings only] max: number
}
```

Returns a table with information about the setting. Throws an exception if the setting does not exist.
