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

-- Returns entity name (string ID).
entities.name(uid: int) -> str

-- Returns a table of all loaded entities
entities.get_all() -> table

-- Returns a table of loaded entities based on the passed list of UIDs
entities.get_all(uids: array<int>) -> table

-- Returns a list of UIDs of entities inside the rectangular area
-- pos - minimal area corner
-- size - area size
entities.get_all_in_box(pos: vec3, size: vec3) -> array<int>

-- Returns a list of UIDs of entities inside the radius
-- center - center of the area
-- radius - radius of the area
entities.get_all_in_radius(center: vec3, radius: number) -> array<int>
```

```lua
entities.raycast(start: vec3, dir: vec3, max_distance: number,
 ignore: int, [optional] destination: table) -> table or nil
```

The function is an extended version of [block.raycast](libblock.md#raycast). Returns a table with the results if the ray touches a block or entity.

Accordingly, this will affect the presence of the *entity* and *block* fields.
