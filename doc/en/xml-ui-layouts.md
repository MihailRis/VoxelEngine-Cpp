# XML UI Building

See also [ui elements in scripting](scripting/ui.md).

# Specific types

**2D vector** - pair of numbers separated with comma.
Examples:
- "500,200"
- "0.4,53.01"
- "0,0"

**3D vector** - three numbers separated with comma.
Examples:
- "60,30,53"
- "0.4,0.1,0.753"

**4D vector** - four numbers separated with comma.
- "10,5,10,3"
- "0.1,0.5,0.0,0.0"

**RGBA color** - only HEX notation available
Examples:
- "#FF8000" - opaque orange
- "#FFFFFF80" - semi-transparent white
- "#000000FF" - opaque black

# Common element attributes

- **id** - element identifier. Type: string.
- **pos** - element position. Type: 2D vector.
- **size** - element size. Type: 2D vector.
- **color** - element color. Type: RGBA color.
- **margin** - element margin. Type: 4D vector
  *left, top, right, bottom*
- **visible** - element visibility. Type: boolean (true/false)
- **position-func** - position supplier for an element (two numbers), called on every parent container size update or on element adding on a container. May be called before *on_hud_open*
# Common *container* attributes

Buttons and panels are also containers.

- **padding** - element padding. Type: 4D vector.
  *left, top, right, bottom*
  **scrollable** - element scrollability. Works on panels only. Type: boolean

# Common *panel* attributes

Buttons are also panels.

- **max-length** - maximal length of panel stretching before scrolling (if scrollable = true). Type: number
# Common elements

## *button*

Inner text is a button text.

- **text-align** - inner text alignment (*left/center/right*). Type: string.
- **onclick** - Lua function called on button press.

## *image*

- **src** - name of an image stored in textures folder. Extension is not specified. Type: string.
  Example: *gui/error*

## *trackbar*

- **min** - minimal value. Type: number. Default: 0
- **max** - maximal value. Type: number. Default: 1
- **value** - initial value. Type: number. Default: 0
- **step** - track step size. Type: number: Default: 1
- **track-width** track pointer width (in steps). Type: number. Default: 1
- **consumer** - Lua function - new value consumer
- **supplier** - Lua function - value supplier

# Inventory elements

## *inventory*

Element is a container. Does not have specific attributes.

> [!WARNING]
> Inventories position is controlled by the engine and can not be changed by attributes *pos* and *margin*

## *slot*

Element must be in direct sub-element of *inventory*.
- **index** - inventory slot index (starting from 0). Type: integer
- **item-source** - content access panel behaviour (infinite source of an item). Type: boolean
- **sharefunc** - Lua event called on <btn>LMB</btn> + <btn>Shift</btn>. Inventory id and slot index passed as arguments.
- **updatefunc** - Lua event called on slot content update.Inventory id and slot index passed as arguments.
- **onrightclick** - Lua event called on <btn>RMB</btn> click. Inventory id and slot index passed as arguments.

## *slots-grid*

- **start-index** - inventory slot index of the first slot. Type: integer
- **rows** - number of grid rows (unnecessary if *cols* and *count* specified). Type: integer
- **cols** - number of grid columns (unnecessary if *rows* and *count* specified). Type: integer
- **count** - total number of slots in grid (unnecessary if *rows* and *cols* specified). Type: integer
- **interval** - visual slots interval. Type: number
- **padding** - grid padding (not slots interval). Type: number. (*deprecated*)
- **sharefunc** - Lua event called on <btn>LMB</btn> + <btn>Shift</btn>. Inventory id and slot index passed as arguments.
- **updatefunc** - Lua event called on slot content update.Inventory id and slot index passed as arguments.
- **onrightclick** - Lua event called on <btn>RMB</btn> click. Inventory id and slot index passed as arguments.

