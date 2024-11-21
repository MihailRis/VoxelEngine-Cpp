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

-- Кодирует код в в UTF-8
utf8.encode(codepoint: int) -> str

-- Возвращает подстроку от позиции startchar до endchar включительно
utf8.sub(text: str, startchar: int, [опционально] endchar: int) -> str

-- Переводит строку в вверхний регистр
utf8.upper(text: str) -> str

-- Переводит строку в нижний регистр
utf8.lower(text: str) -> str

-- Экранирует строку
utf8.escape(text: str) -> str
```
