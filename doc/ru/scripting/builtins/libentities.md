# Библиотека *entities*

Библиотека предназначена для работы с реестром сущностей.

```lua
-- Возвращает сущность по уникальному идентификатору
-- Возвращаемая таблица - та же, что доступна в компонентах сущности.
entities.get(uid: int) -> table

-- Создает указанную сущность.
-- args - таблица таблиц параметров компонентов (переменная ARGS)
-- args не является обязательным
entities.spawn(name: str, pos: vec3, [optional] args: table)

-- Проверяет наличие сущности по уникальному идентификатору.
entities.exists(uid: int) -> bool

-- Возвращает таблицу всех загруженных сущностей
entities.get_all() -> table

-- Возвращает таблицу загруженных сущностей по переданному списку UID
entities.get_all(uids: array<int>) -> table

-- Возвращает список UID сущностей, попадающих в прямоугольную область
-- pos - минимальный угол области
-- size - размер области
entities.get_all_in_box(pos: vec3, size: vec3) -> array<int>

-- Возвращает список UID сущностей, попадающих в радиус
-- center - центр области
-- radius - радиус области
entities.get_all_in_radius(center: vec3, radius: number) -> array<int>
```

```lua
entities.raycast(start: vec3, dir: vec3, max_distance: number,
                 ignore: int, [optional] destination: table) -> table или nil
```

Функция является расширенным вариантом [block.raycast](libblock.md#raycast). Возвращает таблицу с результатами если луч касается блока, либо сущности.

Соответственно это повлияет на наличие полей *entity* и *block*.
