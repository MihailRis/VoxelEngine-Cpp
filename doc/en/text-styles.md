# Text styles

A proprietary Markdown dialect is used to mark up text styles.
Formatting works on UI elements: label and textbox, if `markup="md"` is explicitly specified.

## Styles

| Style         | Example                  | Output                     |
| ------------- | ------------------------ | -------------------------- |
| Bold          | `**Bold font**`          | **Bold font**              |
| Italic        | `*Text in italics*`      | *Text in italics*          |
| Underline     | `__Underlined text__`    | <ins>Underlined text</ins> |
| Strikethrough | `~~Strikethrough text~~` | ~~Strikethrough text~~     |

Styles can be combined. Example:
```md
***__Message__*** using *~~combed~~ combined* styles__~~.~~__
```
Output:

***<ins>Message</ins>*** using *~~combed~~ combined* styles<ins>~~.~~</ins>

# Colors

Text color can be set using a color code: [#RRGGBB]


| Component | Purpose                           |
| --------- | --------------------------------- |
| R         | Represents the intensity of red   |
| G         | Represents the intensity of green |
| B         | Represents the intensity of blue  |

### Example:

<span style="color: #ff0000">
    <span style="color:rgb(105, 105, 105)">
    [#ff0000]
    </span>Red Text
</span>

<span style="color: #00ff00">
    <span style="color:rgb(105, 105, 105)">
    [#00ff00]
    </span>Green Text
</span>

<span style="color: #0000ff">
    <span style="color:rgb(105, 105, 105)">
    [#0000ff]
    </span>Blue Text
</span>
