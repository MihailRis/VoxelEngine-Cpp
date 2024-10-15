# Библиотека *item*

```lua
-- Возвращает строковый id предмета по его числовому id (как block.name)
item.name(itemid: int) -> str

-- Возвращает числовой id предмета по строковому id (как block_index)
item.index(name: str) -> int

-- Возвращает название предмета, отображаемое в интерфейсе.
item.caption(blockid: int) -> str

-- Возвращает максимальный размер стопки для предмета.
item.stack_size(itemid: int) -> int

-- Возвращает общее число доступных предметов (включая сгенерированные)
item.defs_count() -> int

-- Возвращает имя иконки предмета для использования в свойстве 'src' элемента image
item.icon(itemid: int) -> str
```


