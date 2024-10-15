# *cameras* library

Cameras manipulation library.

## Main functions

```lua
cameras.get(name: str) -> table
-- or
cameras.get(index: int) -> table
```

Returns a camera by name or index.

## Camera methods

```lua
-- accessing camera
local cam = cameras.get("pack:camera-name")

-- returns camera index
cam:get_index() -> int

-- returns camera name
cam:get_name() -> str

-- returns camera position
cam:get_pos() -> vec3
-- sets camera position
cam:set_pos(pos:vec3)

-- returns camera rotation
cam:get_rot() -> mat4
-- sets camera rotation
cam:set_rot(rot:mat4)

-- returns camera zoom value
cam:get_zoom() -> number
-- sets camera zoom value
cam:set_zoom(zoom: number)

-- returns angle of the camera's field of view in Y (degrees)
cam:get_fov() -> number
-- sets camera field of view angle in Y (degrees)
cam:set_fov(fov: number)

-- returns true if the Y axis is flipped
cam:is_flipped() -> bool
-- reflects the Y axis when true
cam:set_flipped(flipped: bool)

-- checks if perspective mode is enabled
cam:is_perspective() -> bool
-- turns on/off perspective mode
cam:set_perspective(perspective: bool)

-- returns camera direction vector
cam:get_front() -> vec3
-- returns camera right vector
cam:get_right() -> vec3
-- returns camera up vector
cam:get_up() -> vec3

-- makes camera look to a given point
cam:look_at(point:vec3)

-- makes camera look to a given point with given interpolation factor
cam:look_at(point: vec3, t: number)
```

Use player.set_camera to switch cameras.
