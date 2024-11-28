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
```

Wrappers are not automatically removed without calling `unwrap`.
