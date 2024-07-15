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
    - [entities](scripting/builtins/libentities.md)
    - [mat4](scripting/builtins/libmat4.md)
    - [vec2, vec3, vec4](scripting/builtins/libvecn.md)
- [Модуль core:bit_converter](scripting/modules/core_bit_converter.md)
- [Модуль core:data_buffer](scripting/modules/core_data_buffer.md)
- [Модули core:vector2, core:vector3](scripting/modules/core_vector2_vector3.md)


```lua
require "контентпак:имя_модуля" -- загружает lua модуль из папки modules (расширение не указывается)
```

## Библиотека *pack*

```python
pack.is_installed(packid: str) -> bool
```

Проверяет наличие установленного пака в мире

```python
pack.data_file(packid: str, filename: str) -> str
```

Возвращает путь к файлу данных по типу: `world:data/packid/filename` 
и создает недостающие директории в пути.

Используйте эту функцию при сохранении настроек пака или иных данных в мире.

Пример:
```lua
file.write(pack.data_file(PACK_ID, "example.txt"), text)
```
Для пака *containermod* запишет текст в файл `world:data/containermod/example.txt`

## Библиотека *player*

```python
player.get_pos(playerid: int) -> number, number, number
```

Возвращает x, y, z координаты игрока

```python
player.set_pos(playerid: int, x: number, y: number, z: number)
```

Устанавливает x, y, z координаты игрока

```python
player.get_rot(playerid: int) -> number, number, number
```

Возвращает x, y, z вращения камеры (в радианах)

```python
player.set_rot(playerid: int, x: number, y: number, z: number)
```

Устанавливает x, y вращения камеры (в радианах)

```python
player.get_inventory(playerid: int) -> int, int
```

Возвращает id инвентаря игрока и индекс выбранного слота (от 0 до 9)

```python
player.is_flight() -> bool
player.set_flight(bool)
```

Геттер и сеттер режима полета

```python
player.is_noclip() -> bool
player.set_noclip(bool)
```

Геттер и сеттер noclip режима (выключенная коллизия игрока)

```python
player.set_spawnpoint(playerid: int, x: number, y: number, z: number) 
player.get_spawnpoint(playerid: int) -> number, number, number
```

Сеттер и геттер точки спавна игрока

```python
player.get_selected_block(playerid: int) -> x,y,z
```

Возвращает координаты выделенного блока, либо nil

```python
player.get_selected_entity(playerid: int) -> int
```

Возвращает уникальный идентификатор сущности, на которую нацелен игрок

```python
player.get_entity(playerid: int) -> int
```

Возвращает уникальный идентификатор сущности игрока

## Библиотека *world*

```python
world.get_list() -> массив таблиц {
	name: str,
	icon: str
}
```

Возвращает информацию о мирах: название и предпросмотр (автоматически загружаемая текстура).

```python
world.get_day_time() -> number
```

Возвращает текущее игровое время от 0.0 до 1.0, где 0.0 и 1.0 - полночь, 0.5 - полдень.

```python
world.set_day_time(time: number)
```

Устанавливает указанное игровое время.

```python
world.set_day_time_speed(value: number)
```

Устанавливает указанную скорость для игрового времени.

```python
world.get_day_time_speed() -> number
```

Возвращает скорость для игрового времени.

```python
world.get_total_time() -> number
```

Возвращает общее суммарное время, прошедшее в мире

```python
world.get_seed() -> int
```

Возвращает зерно мира.

```python
world.exists() -> bool
```

Проверяет существование мира по имени.

```python
world.is_day() -> bool
```

Проверяет является ли текущее время днём. От 0.2(8 утра) до 0.8(8 вечера)

```python
world.is_night() -> bool
```

Проверяет является ли текущее время ночью. От 0.8(8 вечера) до 0.2(8 утра)

## Библиотека *pack*

```python
pack.get_folder(packid: str) -> str
```

Возвращает путь к папке установленного контент-пака.

```python
pack.is_installed(packid: str) -> bool
```

Проверяет наличие контент-пака в мире

```python
pack.get_installed() -> массив строк
```

Возращает id всех установленных в мире контент-паков.

```python
pack.get_available() -> массив строк
```

Возвращает id всех доступных, но не установленных в мире контент-паков.

```python
pack.get_base_packs() -> массив строк
```

Возвращает id всех базовых паков (неудаляемых)

```python
pack.get_info(packid: str) -> {
	id: str,
	title: str,
	creator: str,
	description: str,
	version: str,
	icon: str,
	dependencies: опциональный массив строк
}
```

Возвращает информацию о паке (не обязательно установленном).
- icon - название текстуры предпросмотра (загружается автоматически)
- dependencies - строки в формате `{lvl}{id}`, где lvl:
	- `!` - required
	- `?` - optional
	- `~` - weak
	например `!teal`

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

## Библиотека *inventory*

Библиотека функций для работы с инвентарем.

```python
inventory.get(invid: int, slot: int) -> int, int
```

Принимает id инвентаря и индекс слота. Возвращает id предмета и его количество. id = 0 (core:empty) обозначает, что слот пуст.

```python
inventory.set(invid: int, slot: int, itemid: int, count: int)
```

Устанавливает содержимое слота.

```python
inventory.size(invid: int) -> int
```

Возращает размер инвентаря (число слотов). Если указанного инвентаря не существует, бросает исключение.

```python
inventory.add(invid: int, itemid: int, count: int) -> int
```

Добавляет предмет в инвентарь. Если не удалось вместить все количество, возвращает остаток.

```python
inventory.get_block(x: int, y: int, z: int) -> int
```

Функция возвращает id инвентаря указанного блока. Если блок не может иметь инвентарь - возвращает 0.

```python
inventory.bind_block(invid: int, x: int, y: int, z: int)
```

Привязывает указанный инвентарь к блоку.

```python
inventory.unbind_block(x: int, y: int, z: int)
```

Отвязывает инвентарь от блока.

> [!WARNING]
> Инвентари, не привязанные ни к одному из блоков, удаляются при выходе из мира.

```python
inventory.clone(invid: int) -> int
```

Создает копию инвентаря и возвращает id копии. Если копируемого инвентаря не существует, возвращает 0. 

```python
inventory.move(invA: int, slotA: int, invB: int, slotB: int)
```

Перемещает предмет из slotA инвентаря invA в slotB инвентаря invB.
invA и invB могут указывать на один инвентарь.
slotB будет выбран автоматически, если не указывать явно.

## Библиотека item

```python
item.name(itemid: int) -> str
```

Возвращает строковый id предмета по его числовому id (как block.name)

```python
item.index(name: str) -> int
```

Возвращает числовой id предмета по строковому id (как block_index)

```python
item.stack_size(itemid: int) -> int
```

Возвращает максимальный размер стопки для предмета.

```python
item.defs_count() -> int
```

Возвращает общее число доступных предметов (включая сгенерированные)

```python
item.icon(itemid: int) -> str
```

Возвращает имя иконки предмета для использования в свойстве 'src' элемента image

## Библиотека hud

```python
hud.open_inventory()
```

Открывает инвентарь

```python
hud.close_inventory()
```

Закрывает инвентарь

```python
hud.open_block(x: int, y: int, z: int) -> int, str
```

Открывает инвентарь и UI блока. Если блок не имеет макета UI - бросается исключение.

Возвращает id инвентаря блока (при *"inventory-size"=0* создаётся виртуальный инвентарь, который удаляется после закрытия), и id макета UI. 

```python
hud.show_overlay(layoutid: str, playerinv: bool)
```

Показывает элемент в режиме оверлея. Также показывает инвентарь игрока, если playerinv - **true**

> [!NOTE]
> Одновременно может быть открыт только один блок

```python
hud.open_permanent(layoutid: str)
```

Добавляет постоянный элемент на экран. Элемент не удаляется при закрытии инвентаря. Чтобы не перекрывать затенение в режиме инвентаря нужно установить z-index элемента меньшим чем -1. В случае тега inventory, произойдет привязка слотов к инвентарю игрока.

```python
hud.close(layoutid: str)
```

Удаляет элемент с экрана.

```python
hud.get_block_inventory() -> int
```

Дает ID инвентаря открытого блока или 0.

```python
hud.get_player() -> int
```

Дает ID игрока, к которому привязан пользовательский интерфейс.

```python
hud.pause()
```

Открывает меню паузы.

```python
hud.resume()
```

Закрывает меню паузы.

```python
hud.is_paused() -> bool
```

Возвращает true если открыто меню паузы.

```python
hud.is_inventory_open() -> bool
```

Возвращает true если открыт инвентарь или оверлей.

## Библиотека time

```python
time.uptime() -> float
```

Возвращает время с момента запуска движка в секундах.

```python
time.delta() -> float
```

Возвращает дельту времени (время прошедшее с предыдущего кадра)
