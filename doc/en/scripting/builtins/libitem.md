# *item* library

```lua
-- Returns item string ID (name) by index
item.name(itemid: int) -> str

-- Returns item integer ID (index) by name
item.index(name: str) -> int

-- Returns the item display name.
block.caption(blockid: int) -> str

-- Returns max stack size for the item
item.stack_size(itemid: int) -> int

-- Returns count of available item IDs.
item.defs_count() -> int

-- Returns item icon name to use in 'src' property of an image element
item.icon(itemid: int) -> str
```
