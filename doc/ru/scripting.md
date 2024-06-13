# Скриптинг

В качестве языка сценариев используется LuaJIT

Подразделы:
- [События движка](scripting/events.md)
- [Пользовательский ввод](scripting/user-input.md)
- [Файловая система и сериализация](scripting/filesystem.md)
- [Модуль core:bit_converter](scripting/modules/core_bit_converter.md)
- [Модуль core:data_buffer](scripting/modules/core_data_buffer.md)
- [Модули core:Vector2, core:Vector3](scripting/modules/core_Vector2&&Vector3.md)


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
player.get_selected_block(playerid: int) -> x,y,z
```

Возвращает координаты выделенного блока, либо nil

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

## Библиотека *block*

```python
block.name(blockid: int) -> str
```

Возвращает строковый id блока по его числовому id.

```python
block.index(name: str) -> int
```

Возвращает числовой id блока, принимая в качестве агрумента строковый.

```python
block.material(blockid: int) -> str
```

Возвращает id материала блока.

```python
block.caption(blockid: int) -> str
```

Возвращает название блока, отображаемое в интерфейсе.

```python
block.get(x: int, y: int, z: int) -> int
```

Возвращает числовой id блока на указанных координатах. Если чанк на указанных координатах не загружен, возвращает -1.

```python
block.get_states(x: int, y: int, z: int) -> int
```

Возвращает состояние (поворот + доп. информация) в виде целого числа

```python
block.set(x: int, y: int, z: int, id: int, states: int)
```

Устанавливает блок с заданным числовым id и состоянием (0 - по-умолчанию) на заданных координатах.

> [!WARNING]
> `block.set` не вызывает событие on_placed.

```python
block.is_solid_at(x: int, y: int, z: int) -> bool
```

Проверяет, является ли блок на указанных координатах полным

```python
block.is_replaceable_at(x: int, y: int, z: int) -> bool
```
Проверяет, можно ли на заданных координатах поставить блок (примеры: воздух, трава, цветы, вода)

```python
block.defs_count() -> int
```

Возвращает количество id доступных в движке блоков

Следующие три функции используется для учёта вращения блока при обращении к соседним блокам или других целей, где направление блока имеет решающее значение.


```python
block.get_X(x: int, y: int, z: int) -> int, int, int
```

Возвращает целочисленный единичный вектор X блока на указанных координатах с учётом его вращения (три целых числа).
Если поворот отсутствует, возвращает 1, 0, 0

```python
block.get_Y(x: int, y: int, z: int) -> int, int, int
```

Возвращает целочисленный единичный вектор Y блока на указанных координатах с учётом его вращения (три целых числа).
Если поворот отсутствует, возвращает 0, 1, 0

```python
block.get_Z(x: int, y: int, z: int) -> int, int, int
```

Возвращает целочисленный единичный вектор Z блока на указанных координатах с учётом его вращения (три целых числа).
Если поворот отсутствует, возвращает 0, 0, 1

```python
block.get_rotation(x: int, y: int, z: int) -> int
```

Возвращает индекс поворота блока в его профиле вращения.

```python
block.set_rotation(x: int, y: int, z: int, rotation: int)
```

Устанавливает вращение блока по индексу в его профиле вращения.

### Расширенные блоки

Расширенные блоки - те, размер которых превышает 1x1x1

```python
block.is_extended(id: int) -> bool
```

Проверяет, является ли блок расширенным.

```python
block.get_size(id: int) -> int, int, int
```

Возвращает размер блока.

```python
block.is_segment(x: int, y: int, z: int) -> bool
```

Проверяет является ли блок сегментом расширенного блока, не являющимся главным.

```python
block.seek_origin(x: int, y: int, z: int) -> int, int, int
```

Возвращает позицию главного сегмента расширенного блока или исходную позицию,
если блок не являющийся расширенным.

### Пользовательские биты

Выделенная под использования в скриптах часть поля `voxel.states` хранящего доп-информацию о вокселе, такую как вращение блока. На данный момент выделенная часть составляет 8 бит.

```python
block.get_user_bits(x: int, y: int, z: int, offset: int, bits: int) -> int
``` 

Возвращает выбранное число бит с указанного смещения в виде целого беззнакового числа

```python
block.set_user_bits(x: int, y: int, z: int, offset: int, bits: int, value: int) -> int
```
Записывает указанное число бит значения value в user bits по выбранному смещению

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

Удаляет элемент с экрана

```python
hud.get_block_inventory() -> int
```

Дает ID инвентаря открытого блока или 0

```python
hud.get_player() -> int
```

Дает ID игрока, к которому привязан пользовательский интерфейс

```python
hud.pause()
```

Открывает меню паузы

```python
hud.resume()
```

Закрывает меню паузы.

## Библиотека time

```python
time.uptime() -> float
```

Возвращает время с момента запуска движка в секундах

```python
time.delta() -> float
```

Возвращает дельту времени (время прошедшее с предыдущего кадра)
