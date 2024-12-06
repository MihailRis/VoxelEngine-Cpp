# Text styles

A proprietary Markdown dialect is used to mark up text styles.
Formatting works on UI elements: label and textbox, if `markdown="true"` is explicitly specified.

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
