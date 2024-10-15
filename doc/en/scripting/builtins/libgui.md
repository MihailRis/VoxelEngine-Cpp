# *gui* library

The library contains functions for accessing the properties of UI elements. Instead of gui, you should use an object wrapper that provides access to properties through the __index, __newindex meta methods:

Example:

```lua
print(document.some_button.text) -- where 'some_button' is an element id
document.some_button.text = "new text"
```

```python
gui.str(text: str, context: str) -> str
```

Returns translated text.

```python
gui.get_viewport() -> {int, int}
```

Returns size of the main container (window).

```python
gui.get_env(document: str) -> table
```

Returns environment (global variables table) of the specified document.

```lua
gui.get_locales_info() -> table of tables {
  name: str
 }
-- where
--   key - locale id following isolangcode_ISOCOUNTRYCODE format
--   value - table {
--       name: str # locale display name
--   }
```

Returns information about all loaded locales (res/texts/\*).
