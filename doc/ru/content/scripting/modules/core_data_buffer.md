# Модуль core:data_buffer

## Буффер данных
### Хранит в себе массив байтов и позволяет легко получать или добавлять разные значения

```lua
function data_buffer(bytes)
```
Создаёт новый экземпляр data_buffer (параметр bytes необязательный)

```lua
function data_buffer:put_byte(integer: byte)
```
Записывает байт в буффер

```lua
function data_buffer:put_bytes(table: bytes)
```
Записывает байты в буффер

```lua
function data_buffer:put_string(string: str)
```
Конвертирует строку в байты и записывает их в буффер

```lua
function data_buffer:put_bool(boolean: bool)
```
Конвертирует булевое значение в байт и записывает его в буффер

```lua
function data_buffer:put_single(number: single)
```
Конвертирует плавающее число одинарной точности в байты и записывает их в буффер

```lua
function data_buffer:put_double(number: double)
```
Конвертирует плавающее число двойной точности в байты и записывает их в буффер

```lua
function data_buffer:put_uint16(integer: int)
```
Конвертирует беззнаковое 2-х байтовое число в байты и записывает их в буффер

```lua
function data_buffer:put_uint32(integer: int)
```
Конвертирует беззнаковое 4-х байтовое число в байты и записывает их в буффер

```lua
function data_buffer:put_int16(integer: int)
```
Конвертирует знаковое 2-х байтовое число в байты и записывает их в буффер

```lua
function data_buffer:put_int32(integer: int)
```
Конвертирует знаковое 4-х байтовое число в байты и записывает их в буффер

```lua
function data_buffer:put_int64(integer: int)
```
Конвертирует знаковое 8-и байтовое число в байты и записывает их в буффер

```lua
function data_buffer:put_number(number: num)
```
Конвертирует любое число в байты и записывает их в буффер;

Первый байт это тип значения:
```lua
zero = 0
uint16 = 1
uint32 = 2
int16 = 3
int32 = 4
int64 = 5
double = 6
```

```lua
function data_buffer:get_byte() -> integer
```
Возвращает следующий байт из буффера

```lua
function data_buffer:get_bytes(n) -> table
```
Возвращает n следующих байтов, если n равен nil или не указан, то возвращается массив всех байтов

```lua
function data_buffer:get_string() -> string
```
Читает следующую строку из буффера

```lua
function data_buffer:get_bool() -> boolean
```
Читает следующий логический булев из буффера

```lua
function data_buffer:get_single() -> number
```
Читает следующее плавающее число одинарной точности из буффера

```lua
function data_buffer:get_double() -> number
```
Читает следующее плавающее число двойной точности из буффера

```lua
function data_buffer:get_uint16() -> integer
```
Читает следующее 2-х байтовое беззнаковое целое число из буффера

```lua
function data_buffer:get_uint32() -> integer
```
Читает следующее 4-х байтовое беззнаковое целое число из буффера

```lua
function data_buffer:get_int16() -> integer
```
Читает следующее 2-х байтовое знаковое целое число из буффера

```lua
function data_buffer:get_int32() -> integer
```
Читает следующее 4-х байтовое знаковое целое число из буффера

```lua
function data_buffer:get_int64() -> integer
```
Читает следующее 8-х байтовое знаковое целое число из буффера

```lua
function data_buffer:get_number() -> number
```
Читает следующее число (см. data_buffer:put_number)

```lua
function data_buffer:size() -> integer
```
Возвращает размер буффера

```lua
function data_buffer:set_position(integer: pos)
```
Устанавливает текущую позицию в буффере

```lua
function data_buffer:set_bytes(table: bytes)
```
Устанавливает байты в буффер
