# Скриптинг

В качестве языка сценариев используется LuaJIT

Подразделы:
- [События движка](scripting/events.md)
- [Пользовательский ввод](scripting/user-input.md)
- [Файловая система и сериализация](scripting/filesystem.md)
- [Свойства и методы UI элементов](scripting/ui.md)
- [Сущности и компоненты](scripting/ecs.md)
- [Библиотеки](#)
    - [block](scripting/builtins/libblock.md)
    - [cameras](scripting/builtins/libcameras.md)
    - [entities](scripting/builtins/libentities.md)
    - [hud](scripting/builtins/libhud.md)
    - [inventory](scripting/builtins/libinventory.md)
    - [item](scripting/builtins/libitem.md)
    - [mat4](scripting/builtins/libmat4.md)
    - [pack](scripting/builtins/libpack.md)
    - [player](scripting/builtins/libplayer.md)
    - [quat](scripting/builtins/libquat.md)
    - [vec2, vec3, vec4](scripting/builtins/libvecn.md)
    - [world](scripting/builtins/libworld.md)
- [Модуль core:bit_converter](scripting/modules/core_bit_converter.md)
- [Модуль core:data_buffer](scripting/modules/core_data_buffer.md)
- [Модули core:vector2, core:vector3](scripting/modules/core_vector2_vector3.md)

## Аннотации типов данных

В документации к Lua библиотекам используются аннотации типов,
не являющиеся частью синтаксиса Lua.

- vector - массив из трех или четырех чисел
- vec2 - массив из двух чисел
- vec3 - массив из трех чисел
- vec4 - массив из четырех чисел
- quat - массив из четырех чисел - кватернион
- matrix - массив из 16 чисел - матрица

```lua
require "контентпак:имя_модуля" -- загружает lua модуль из папки modules (расширение не указывается)
```

## Библиотека *gui*

Библиотека содержит функции для доступа к свойствам UI элементов. Вместо gui следует использовать объектную обертку, предоставляющую доступ к свойствам через мета-методы __index, __newindex:

```lua
print(document.some_button.text) -- где 'some_button' - id элемета
document.some_button.text = "новый текст"
```

В скрипте макета `layouts/файл_макета.xml` - `layouts/файл_макета.xml.lua` уже доступна переменная **document** содержащая объект класса Document

```python
gui.str(text: str, context: str) -> str
```

Возращает переведенный текст.

```python
gui.get_viewport() -> {int, int}
```

Возвращает размер главного контейнера (окна).

```python
gui.get_env(document: str) -> table
```

Возвращает окружение (таблица глобальных переменных) указанного документа.

```python
get_locales_info() -> таблица таблиц где
   ключ - id локали в формате isolangcode_ISOCOUNTRYCODE
   значение - таблица {
       name: str # название локали на её языке
   }
```

Возвращает информацию о всех загруженных локалях (res/texts/\*).

## Библиотека time

```python
time.uptime() -> float
```

Возвращает время с момента запуска движка в секундах.

```python
time.delta() -> float
```

Возвращает дельту времени (время прошедшее с предыдущего кадра)
