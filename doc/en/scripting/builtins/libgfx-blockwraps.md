# Library *gfx.blockwraps*

Library for working with *block wrappers*.

Block wrappers are introduced to implement block destruction animation and can be used for other purposes.

```lua
-- Creates a wrapper at the specified position, with the specified texture.
-- Returns the wrapper id.
gfx.blockwraps.wrap(position: vec3, texture: str) --> int

-- Removes the wrapper, if it exists.
gfx.blockwraps.unwrap(id: int)

-- Changes the position of the wrapper, if it exists.
gfx.blockwraps.set_pos(id: int, position: vec3)

-- Changes the texture of the wrapper, if it exists.
gfx.blockwraps.set_texture(id: int, texture: str)

-- Gets the wrapper position if it exists.
-- Returns the wrapper position.
gfx.blockwraps.get_texture(id: int)

-- Gets the texture of the wrapper, if it exists.
-- Returns the wrapper texture.
gfx.blockwraps.get_texture(id: int)

-- Checks the existence of the wrapper.
-- Returns true/false depending on the wrapper existence.
gfx.blockwraps.is_alive(id: int)

-- Searches for wrappers at the specified position.
-- Returns an array of wrapper ids.
gfx.block wrap s.get_on_pos(position: vec3)

-- Removes all wrappers at the specified position.
gfx.blockwraps.unwrap_on_pos(position: vec3)
```

Wrappers are removed automatically when the block is broken/replaced.
