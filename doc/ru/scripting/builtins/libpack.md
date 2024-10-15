# Библиотека *pack*

```python
pack.is_installed(packid: str) -> bool
```

Проверяет наличие установленного пака в мире

```lua
pack.data_file(packid: str, filename: str) -> str
-- и
pack.shared_file(packid: str, filename: str) -> str
```

Возвращает путь к файлу данных 
и создает недостающие директории в пути.

- Первый вариант возвращает: `world:data/packid/filename`
- Второй вариант возвращает: `config:packid/filename`

Примеры:
```lua
file.write(pack.data_file(PACK_ID, "example.txt"), text)
```
Для пака *containermod* запишет текст в файл `world:data/containermod/example.txt`.

Используйте для хранения данных в мире.

```lua
file.write(pack.shared_file(PACK_ID, "example.txt"), text)
```
Для пака *containermod* запишет текст в файл `config:containermod/example.txt`

Используйте для хранения данныхm общих для всех миров.

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
