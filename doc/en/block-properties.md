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

## Physics

### *obstacle*

Block is not a physical obstacle if **false**

### *hitbox*

An array of 6 numbers describing an offset an size of a block hitbox.

Array *\[0.25, 0.0, 0.5,  0.75, 0.4, 0.3\]* describes hitbox width:
- 0.75m width (from east to west)
- 0.4m height
- 0.3m length (from south to north)
- offset 0.25m east
- offset 0.0m up
- offset 0.5m north

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
