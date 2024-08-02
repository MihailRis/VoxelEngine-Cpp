# Пользовательский ввод

Обработка нажатий клавиш и кнопок мыши обрабатываются через привязки (bindings), которые назначаются в паке, в файле `config/bindings.toml` в формате:

```toml
packid.binding.name="inputtype:codename"
```

- packid - опционально, но желательно
- inputtype - key или mouse
- codename - имя клавиши или кнопки мыши (left/right/middle)

## Имена клавиш

- space, backspace, tab, enter, caps-lock, escape
- left-ctrl, left-shift, left-alt, left-super
- right-ctrl, right-shift, right-alt, right-super
- delete, home, end, insert, page-up, page-down
- left, right, down, up
- a..z
- 0..9
- f1..f25

## Библиотека input

```python
input.keycode(keyname: str) -> int
```

Возвращает код клавиши по имени, либо -1

```python
input.mousecode(mousename: str) -> int
```

Возвращает код кнопки мыши по имени, либо -1

```python
input.add_callback(bindname: str, callback: function)
```

Назначает функцию, которая будет вызываться при активации привязки. Пример:
```lua
input.add_callback("hud.inventory", function ()
	print("Inventory open key pressed")
end)
```

```python
input.get_mouse_pos() -> {int, int}
```

Возвращает позицию курсора на экране.

```python
input.get_bindings() -> массив строк
```

Возвращает названия всех доступных привязок.

```python
input.is_active(bindname: str) -> bool
```

Проверяет активность привязки.

```python
input.is_pressed(code: str) -> bool
```

Проверяет активность ввода по коду, состоящему из:
- типа ввода: key (клавиша) или mouse (кнопка мыши)
- код ввода: [имя клавиши](#имена-клавиш) или имя кнопки мыши (left, middle, right)

Пример: 
```lua
if input.is_pressed("key:enter") then
    ...
end
```
