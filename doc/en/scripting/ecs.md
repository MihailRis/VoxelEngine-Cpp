# Entities and components

## Types notation used below

- vec3 - 3D vector (array of three numbers)
- mat4 - 4x4 matrix (array of 16 numbers)

Type annotations are added for documentation purposes and are not part of the Lua syntax.

## Entity

The entity object is available in components as a global variable **entity**.

```lua
-- Deletes an entity (the entity may continue to exist until the frame ends, but will not be displayed in that frame)
entity:despawn()

-- Returns the name of the entity skeleton
entity:get_skeleton() -> str
-- Replaces the entity skeleton
entity:set_skeleton(name: str)

-- Returns the unique entity identifier
entity:get_uid() -> int

-- Returns the component by name
entity:get_component(name: str) -> component or nil
-- Checks for the presence of a component by name
entity:has_component(name: str) -> bool
```

## Built-in components

### Transform

The component is responsible for the position, scale and rotation of the entity.

```lua
-- Alias
local tsf = entity.transform

-- Returns the position of the entity
tsf:get_pos() -> vec3
-- Sets the entity position
tsf:set_pos(pos:vec3)

-- Returns the entity scale 
tsf:get_size() -> vec3
-- Sets the entity scale
tsf:set_size(size: vec3)

-- Returns the entity rotation
tsf:get_rot() -> mat4
-- Sets entity rotation
tsf:set_rot(size: mat4)
```

### Rigidbody

The component is responsible for the physical body of the entity.

```lua
-- Alias
local body = entity.rigidbody

-- Checks if body physics calculation is enabled
body:is_enabled() -> bool
-- Enables/disables body physics calculation
body:set_enabled(enabled: bool)

-- Returns linear velocity
body:get_vel() -> vec3
-- Sets linear velocity
body:set_vel(vel: vec3)

-- Returns the size of the hitbox
body:get_size() -> vec3
-- Sets the hitbox size 
body:set_size(size: vec3)

-- Returns the gravity multiplier
body:get_gravity_scale() -> vec3
-- Sets the gravity multiplier
body:set_gravity_scale(scale: vec3)

-- Returns the linear velocity attenuation multiplier (used to simulate air resistance and friction)
body:get_linear_damping() -> number
-- Sets the linear velocity attenuation multiplier
body:set_linear_damping(value: number)

-- Checks if vertical velocity attenuation is enabled
body:is_vdamping() -> bool
-- Enables/disables vertical velocity attenuation
body:set_vdamping(enabled: bool)

-- Checks if the entity is on the ground
body:is_grounded() -> bool

-- Checks if the entity is in a "crouching" state (cannot fall from blocks)
body:is_crouching() -> bool
-- Enables/disables the "crouching" state
body:set_crouching(enabled: bool)

-- Returns the type of physical body (dynamic/kinematic)
body:get_body_type() -> str
-- Sets the physical body type
body:set_body_type(type: str)
```

### Skeleton

The component is responsible for the entity skeleton. See [rigging](../rigging.md).

```lua
-- Alias
local rig = entity.skeleton

-- Returns the model name assigned to the bone at the specified index
rig:get_model(index: int) -> str

-- Returns the bone transformation matrix at the specified index
rig:get_matrix(index: int) -> mat4
-- Sets the bone transformation matrix at the specified index
rig:set_matrix(index: int, matrix: mat4)

-- Returns the texture by key (dynamically assigned textures - '$name')
rig:get_texture(key: str) -> str

-- Assigns texture by key
rig:set_texture(key: str, value: str)

-- Returns the bone index by name or nil
rig:index(name: str) -> int
```
