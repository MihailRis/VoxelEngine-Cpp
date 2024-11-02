# User input

User input is performed with *bindings* defined with `config/bindings.toml`.

Example:

```toml
packid.binding.name="inputtype:codename"
```

- packid - optional (recommended)
- inputtype - key or mouse
- codename - key or mouse button code (left/right/middle)

## Key names

- space, backspace, tab, enter, caps-lock, escape
- left-ctrl, left-shift, left-alt, left-super
- right-ctrl, right-shift, right-alt, right-super
- delete, home, end, insert, page-up, page-down
- left, right, down, up
- a..z
- 0..9
- f1..f25

## *input* library

```python
input.keycode(keyname: str) -> int
```

Returns key code or -1 if unknown

```python
input.mousecode(mousename: str) -> int
```

Returns mouse button code or -1 if unknown

```python
input.add_callback(bindname: str, callback: function)
```

Add binding activation callback. Example:
```lua
input.add_callback("hud.inventory", function ()
	print("Inventory open key pressed")
end)
```

```python
input.get_mouse_pos() -> {int, int}
```

Returns cursor screen position.

```python
input.get_bindings() -> strings array
```

Returns all binding names.

```python
input.get_binding_text(bindname: str) -> str
```

Returns text representation of button by binding name.

```python
input.is_active(bindname: str) -> bool
```

Checks if the binding is active.

```python
input.set_enabled(bindname: str, flag: bool)
```

Enables/disables binding until leaving the world.

```python
input.is_pressed(code: str) -> bool
```

Checks input activity using a code consisting of:
- input type: *key* or *mouse*
- input code: [key name](#key names) or mouse button name (left, middle, right)

Example:
```lua
if input.is_pressed("key:enter") then
    ...
end
```
