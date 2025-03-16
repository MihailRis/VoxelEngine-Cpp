# *inventory* library

Library for inventories interaction.

```lua
-- ID = 0 (core:empty) means that slot is empty.
inventory.get(
    -- inventory ID
    invid: int,
    -- slot index
    slot: int
) -> int, int
-- Returns item ID and count. 

-- Set slot content, deleting the data contained before.
inventory.set(
    -- inventory ID
    invid: int,
    -- slot index
    slot: int,
    -- item index
    itemid: int,
    -- item count
    count: int
)

-- Sets the count of items in the slot without affecting the data if the argument is non-zero.
inventory.set_count(
    -- inventory ID
    invid: int,
    -- slot index
    slot: int,
    -- item count
    count: int
)

-- Checks for the presence of a local property by name without copying its value.
-- Preferably for tables, but not primitive types.
inventory.has_data(
    -- inventory ID
    invid: int,
    -- slot index
    slot: int,
    -- property name
    name: str
) -> bool

-- Returns a copy of value of a local property of an item by name or nil.
inventory.get_data(
    -- inventory ID
    invid: int,
    -- slot index
    slot: int,
    -- property name
    name: str
) -> any

-- Sets the value of a local property of an item by name.
-- Nil value removes the property.
inventory.set_data(
    -- inventory ID
    invid: int,
    -- slot index
    slot: int,
    -- property name
    name: str
    -- value
    value: any
)

-- Returns a copy of the table of all local item properties.
inventory.get_all_data(
    -- inventory ID
    invid: int,
    -- slot index
    slot: int,
) -> table

-- Returns inventory size (slots number). 
-- Throws an exception if there's no inventory having specified ID.
inventory.size(invid: int) -> int

-- Add an item to the specified inventory. 
-- Returns remaining count if could not to add fully.
inventory.add(invid: int, itemid: int, count: int) -> int

-- Returns the index of the first matching slot in the given range.
-- If no matching slot was found, returns nil
inventory.find_by_item(
    -- inventory id
    invid: int,
    -- item id
    itemid: int,
    -- [optional] index of the slot range start (from 0)
    range_begin: int,
    -- [optional] index of the slot range end (from 0)
    range_end: int,
    -- [optional] minimum item count in the slot
    min_count: int = 1
) -> int

-- Returns block inventory ID or 0.
inventory.get_block(x: int, y: int, z: int) -> int

-- Bind inventory to the specified block.
inventory.bind_block(invid: int, x: int, y: int, z: int)

-- Unbind inventory from the specified block.
inventory.unbind_block(x: int, y: int, z: int)

-- Remove inventory.
inventory.remove(invid: int)
```

> [!WARNING]
> Unbound inventories will be deleted on world close.

```lua
-- Create inventory. Returns the created ID.
inventory.create(size: int) -> int

-- Create inventory copy. Returns the created copy ID.
inventory.clone(invid: int) -> int

-- Move an item from slotA of invA to slotB of invB. 
-- invA may be the same as invB.
-- If slotB will be chosen automaticly if argument is not specified.
-- The move may be incomplete if the available slot has no enough stack space.
inventory.move(invA: int, slotA: int, invB: int, slotB: int)

-- Moves an item from slotA of inventory invA to a suitable slot(s)
-- in the specified range of inventory invB.
-- invA may be the same as invB.
-- The move may be incomplete if the available slots are filled.
inventory.move(
    invA: int, 
    slotA: int, 
    invB: int, 
    rangeBegin: int, 
    [optional] rangeEnd: int
)
```
