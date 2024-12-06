# Библиотека *gui*

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
gui.get_locales_info() -> таблица таблиц где
   ключ - id локали в формате isolangcode_ISOCOUNTRYCODE
   значение - таблица {
       name: str # название локали на её языке
   }
```

Возвращает информацию о всех загруженных локалях (res/texts/\*).

```lua
gui.clear_markup(
    -- язык разметки ("md" - Markdown)
    language: str, 
    -- текст с разметкой
    text: str
) -> str
```

Удаляет разметку из текста.
