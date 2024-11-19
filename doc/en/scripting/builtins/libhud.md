# *hud* library

```lua
-- Open player inventory.
hud.open_inventory()

-- Close inventory.
hud.close_inventory()

-- Open UI and inventory.
-- Throws an exception if has no UI layout.
hud.open(invid: int, layoutid: str)

-- Open block UI and inventory.
-- Throws an exception if block has no UI layout.
-- Returns block inventory ID (if *"inventory-size"=0* a virtual 
-- inventory will be created), and UI layout ID.
hud.open_block(x: int, y: int, z: int) -> int, str
```

> [!NOTE]
> Only one block may be open at same time

```lua
-- Show overlay with layout specified.
-- Shows player inventory also if playerinv is true.
hud.show_overlay(layoutid: str, playerinv: bool)

-- Add element to the screen. 
-- The element will be removed on world close only.
-- inventory element will be bound to the player inventory.
hud.open_permanent(layoutid: str)

-- Remove an element from the screen.
hud.close(layoutid: str)

-- Get open block inventory ID or 0.
hud.get_block_inventory() -> int

-- Gives the ID of the player that the UI is bound to.
hud.get_player() -> int

-- Opens the pause menu
hud.pause()

-- Closes the pause menu.
hud.resume()

-- Returns true if pause menu is open.
hud.is_paused() -> bool

-- Returns true if inventory is open or overlay is shown.
hud.is_inventory_open() -> bool
```
