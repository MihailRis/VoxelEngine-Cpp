# UI properties and methods

UI elements in scripts are accessed through a Document instance
(*document* variable) by id specified in xml.

Example: print the pos property of an element with id: "worlds-panel" to the console:
```lua
print(document["worlds-panel"].pos)
-- or
local worldsPanel = document["worlds-panel"]
print(worldsPanel.pos)
```

The IDs of the elements are global for the document, that is, *worlds-panel* can be located either in the root element,
and in a nested container.

The element id cannot be changed from a script.

The following tables will use abbreviated type descriptions, such as:
- vec2 - an array of two numbers.
- ivec2 - an array of two integers.
- rgba - an array of four integers in the range `[0..255]` denoting RGBA constituent colors.

Element methods, according to OOP features in Lua, are called using the `:` operator instead of `.`

For example:
```lua
document["worlds-panel"]:clear()
```

## General properties and methods

Properties that apply to all elements:

| Name          | Type   | Read | Write | Description                                 |
| ------------- | ------ | ---- | ----- | ------------------------------------------- |
| id            | string | yes  | *no*  | element id                                  |
| pos           | vec2   | yes  | yes   | element position inside a container         |
| wpos          | vec2   | yes  | yes   | element position inside the window          |
| size          | vec2   | yes  | yes   | element size                                |
| interactive   | bool   | yes  | yes   | ability to interact with the element        |
| enabled       | bool   | yes  | yes   | visually indicated version of *interactive* |
| visible       | bool   | yes  | yes   | element visibility                          |
| focused       | bool   | yes  | yes   | focus on element                            |
| color         | rgba   | yes  | yes   | element color                               |
| hoverColor    | rgba   | yes  | yes   | hover color                                 |
| pressedColor  | rgba   | yes  | yes   | color when pressed                          |
| tooltip       | string | yes  | yes   | tooltip text                                |
| tooltipDelay  | float  | yes  | yes   | tooltip delay                               |
| contentOffset | vec2   | yes  | *no*  | element content offset                      |
| cursor        | string | yes  | yes   | cursor displayed on hover                   |

Common element methods:

| Method              | Description                                                                         |
| ------------------- | ----------------------------------------------------------------------------------- |
| moveInto(container) | moves the element to the specified container (the element is specified, not the id) |
| destruct()          | removes element                                                                     |

## Containers

Common methods for containers (elements: container, panel, button, pagebox):

| Method                          | Description                                                                                  |
| ------------------------------- | -------------------------------------------------------------------------------------------- |
| clear()                         | clears content                                                                               |
| add(xml)                        | adds an element, creating it using xml code. Example: `container:add("<image src='test'/>")` |
| setInterval(interval, callback) | assigns a function to be executed repeatedly at an interval specified in milliseconds        |

## Textbox

Properties:

| Name        | Type   | Read | Write | Description                                                                          |
| ----------- | ------ | ---- | ----- | ------------------------------------------------------------------------------------ |
| text        | string | yes  | yes   | entered text or placeholder                                                          |
| placeholder | string | yes  | yes   | placeholder (used if nothing has been entered)                                       |
| hint        | string | yes  | yes   | text to display when nothing is entered                                              |
| caret       | int    | yes  | yes   | carriage position. `textbox.caret = -1` will set the position to the end of the text |
| editable    | bool   | yes  | yes   | text mutability                                                                      |
| multiline   | bool   | yes  | yes   | multiline support                                                                    |
| lineNumbers | bool   | yes  | yes   | display line numbers                                                                 |
| textWrap    | bool   | yes  | yes   | automatic text wrapping (only with multiline: "true")                                |
| valid       | bool   | yes  | no    | is the entered text correct                                                          |
| textColor   | vec4   | yes  | yes   | text color                                                                           |
| syntax      | string | yes  | yes   | syntax highlighting ("lua" - Lua)                                                    |
| markup      | string | yes  | yes   | text markup language ("md" - Markdown)                                               |

Methods:

| Method                    | Description                                                      |
| ------------------------- | ---------------------------------------------------------------- |
| paste(text: str)          | inserts the specified text at the caret position                 |
| lineAt(pos: int) -> int   | determines the line number by position in the text               |
| linePos(line: int) -> int | determines the position of the beginning of the line in the text |

## Slider (trackbar)

Properties:

| Name       | Type  | Read | Write | Description           |
| ---------- | ----- | ---- | ----- | --------------------- |
| value      | float | yes  | yes   | current value         |
| min        | float | yes  | yes   | minimum value         |
| max        | float | yes  | yes   | maximum value         |
| step       | float | yes  | yes   | division step         |
| trackWidth | float | yes  | yes   | control element width |
| trackColor | rgba  | yes  | yes   | control element color |

## Menu (pagebox)

Properties:

| Name  | Type   | Read | Write | Description  |
| ----- | ------ | ---- | ----- | ------------ |
| page  | string | yes  | yes   | current page |

Methods:

| Method  | Description                       |
| ------- | --------------------------------- |
| back()  | switches to previous page         |
| reset() | resets page and switching history |

## Checkbox

Properties:

| Name    | Type | Read | Write | Description |
| ------- | ---- | ---- | ----- | ----------- |
| checked | bool | yes  | yes   | mark status |

## Button

Properties:

| Name  | Type   | Read | Write | Description  |
| ----- | ------ | ---- | ----- | ------------ |
| text  | string | yes  | yes   | button text  |

## Label

Properties:

| Name   | Type   | Read | Write | Description                            |
| ------ | ------ | ---- | ----- | -------------------------------------- |
| text   | string | yes  | yes   | label text                             |
| markup | string | yes  | yes   | text markup language ("md" - Markdown) |

## Image

Properties:

| Name  | Type   | Read | Write | Description  |
| ----- | ------ | ---- | ----- | ------------ |
| src   | string | yes  | yes   | texture name |

## Inventory

Properties:

| Name      | Type | Read | Write | Description                                       |
| --------- | ---- | ---- | ----- | ------------------------------------------------- |
| inventory | int  | yes  | yes   | id of the inventory to which the element is bound |
