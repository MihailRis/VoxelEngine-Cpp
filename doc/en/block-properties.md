# Block properties

## Visual

### *texture*

Block texture name (name of the file in `textures/blocks/` with no path and extension included, just name)

Texture file must be a **png** image

### *texture-faces*

> [!IMPORTANT]
> Can't be used if `texture` already specified

An array of 6 texture names for block sides.

Example:
```json
"texture-faces": [
    "grass_side",
    "grass_side",
    "dirt",
    "grass_top",
    "grass_side",
    "grass_side"
]
```

### *model*

Block model type from list:
- "block" - default block model
- "none" - invisible block (air)
- "X" - grass model (two crossed sprites)
- "aabb" - model based of block hitbox (complex hitbox will be combined into one). Examples: pipes, bulbs, panels.

### *draw-group*

Integer specifying number of block draw group (render order). Used for semi-transparent blocks.

### *translucent*

Enables translucency support in block textures (examples: water, ice).
Should only be used when needed, as it impacts performance.
Not required for full transparency (grass, flowers).

### *rotation*

Rotation profile (set of available block rotations and behaviour of placing block rotation) from list:

- "none" - no rotation available (default profile)
- "pipe" - wood logs, pipes, pillars
- "pane" - panels, doors, signs

## Lighting

### *emission*

An array of 3 integers - R, G, B of light in range \[0, 15\]

Examples:

- *\[15, 15, 15\]* - white with maximal intensity
- *\[7, 0, 0\]* - dim red light
- *\[0, 0, 0\]* - no emission (default value)

### *light-passing*

Light ignores block if **true**

### *sky-light-passing*

Vertical sky light ray ignores block if **true**. (used for water)

### *shadeless*

Turns off block model shading

### *ambient-occlusion* (Vertex-based Ambient-Occlusion)

Determines the presence of the vertex AO effect. Turned-on by default.

### *culling*

Face culling mode:
- **default** - normal face culling
- **optional** - face culling among blocks of the same rendering group can be disabled via the `graphics.dense-render` setting.
- **disabled** - face culling among blocks of the same rendering group disabled.

## Physics

### *obstacle*

Block is not a physical obstacle if **false**

### *hitbox*

An array of 6 numbers describing an offset an size of a block hitbox.

Array *\[0.25, 0.0, 0.5,  0.75, 0.4, 0.3\]* describes hitbox width:
- offset 0.25m east
- offset 0.0m up
- offset 0.5m north
- 0.75m width (from east to west)
- 0.4m height
- 0.3m length (from south to north)

### *grounded*

Is block may only be set on a solid block and destructs on below block destruction.

### *selectable*

Cursor ray will ignore block if **false**.

### *replaceable*

Is block replaceable. Examples: air, water, grass, flower.

### *breakable*

Is block breakable by mouse click.

## Inventory

### *hidden*

If **true** an item will not be generated for block. **picking-item** must be specified

### *picking-item*

Item will be chosen on MMB click on the block.

Example: block `door:door_open` is hidden, so you need to specify `picking-item: "door:door.item"` to bind it to not hidden `door:door` block item.

### *script-name*

Used to specify block script name (to reuse one script to multiple blocks). Name must not contain `packid:scripts/` and extension. Just name.

### *ui-layout*

Block UI XML layout name. Default: string block id.

Examples for block `containermod:container`:
- default: `containermod:container` (*containermod/layouts/container.xml*)
- if `containermod:randombox` specified: (*containermod/layouts/randombox.xml*)

### *inventory-size*

Number of block inventory slots. Default - 0 (no inventory).

## Extended blocks

### *size*

Array of three integers. Default value is `[1, 1, 1]`.

## Block fields

Block fields allow you to write more data unique to a specified voxel than the user bits allow.

Block fields are declared in the following format:

```json
"fields": {
    "name": {"type": "data_type"},
    "array_name": {"type": "data_type", "length": "array_length"}
}
```

In addition to `type` and `length`, the `convert-strategy` parameter determines the value conversion strategy when narrowing the data type.

The parameter takes one of two values:
- `reset` - a value that does not exists in the new range will be reset to 0
- `clamp` - the value will be reduced to the closest one in the new range

Example: the number 231 when changing the field type from int16 to int8:
- in `reset` mode will turn into 0
- in `clamp` mode will turn into 127

Available data types:

| Type    | Size      | Description            |
| ------- | --------- | ---------------------- |
| int8    | 1 byte    | signed integer 8 bits  |
| int16   | 2 bytes   | signed integer 16 bits |
| int32   | 4 bytes   | signed integer 32 bits |
| int64   | 8 bytes   | integer signed 64 bits |
| float32 | 4 bytes   | floating-point 32 bits |
| float64 | 8 bytes   | floating-point 64 bits |
| char    | 1 byte    | character              |

- Currently, the total sum of the field sizes cannot exceed 240 bytes.
- A field without an array length specification is equivalent to an array of 1 element.
- A character array can be used to store UTF-8 strings.

## User properties

User properties must be declared in `pack:config/user-props.toml` file:
```toml
"pack:property_name" = {}
```

Example: [user properties of pack **base**](../../res/content/base/config/user-props.toml).
