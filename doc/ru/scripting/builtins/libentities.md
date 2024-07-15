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
```

```lua
entities.raycast(start: vec3, dir: vec3, max_distance: number,
                 ignore: int, [optional] destination: table) -> table или nil
```

Функция является расширенным вариантом [block.raycast](libblock.md#raycast). Возвращает таблицу с результатами если луч касается блока, либо сущности.

Соответственно это повлияет на наличие полей *entity* и *block*.
