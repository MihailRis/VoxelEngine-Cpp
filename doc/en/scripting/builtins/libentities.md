# Library *entities*

The library is designed to work with a registry of entities.

```lua
-- Returns an entity by unique identifier
-- The table returned is the same one available in the entity components.
entities.get(uid: int) -> table

-- Creates the specified entity.
-- args - table of component parameter tables (ARGS variable)
-- args is optional
entities.spawn(name: str, pos: vec3, [optional] args: table)

-- Checks the existence of an entity by a unique identifier.
entities.exists(uid: int) -> bool
```

```lua
entities.raycast(start: vec3, dir: vec3, max_distance: number,
 ignore: int, [optional] destination: table) -> table or nil
```

The function is an extended version of [block.raycast](libblock.md#raycast). Returns a table with the results if the ray touches a block or entity.

Accordingly, this will affect the presence of the *entity* and *block* fields.
