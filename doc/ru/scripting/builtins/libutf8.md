# Библиотека *utf8*

Библиотека предоставляет функции для работы с UTF-8.

```lua
-- Конвертирует UTF-8 строку в Bytearray или массив чисел если
-- второй аргумент - true
utf8.tobytes(text: str, [опционально] usetable=false) -> Bytearray|table

-- Конвертирует Bytearray или массив чисел в UTF-8 строку
utf8.tostring(bytes: Bytearray|table) -> str

-- Возвращает длину юникод-строки
utf8.length(text: str) -> int

-- Возвращает код первого символа строки
utf8.codepoint(chars: str) -> int
```
