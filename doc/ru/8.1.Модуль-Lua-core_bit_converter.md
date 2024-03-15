## Конвертация значений в байты и обратно

```lua
function bit_converter.string_to_bytes(string: str) -> table
```
Конвертирует строку в байты

```lua
function bit_converter.bool_to_byte(boolean: bool) -> integer
```
Конвертирует логический булев в байт

```lua
function bit_converter.single_to_bytes(number: single) -> table
```
Конвертирует плавающее значение одинарной точности в байты

```lua
function bit_converter.double_to_bytes(number: double) -> table
```
Конвертирует плавающее значение двойной точности в байты

```lua
function bit_converter.uint16_to_bytes(integer: int) -> table
```
Конвертирует беззнаковое 2-х битное целое число в байты

```lua
function bit_converter.uint32_to_bytes(integer: int) -> table
```
Конвертирует беззнаковое 4-х битное целое число в байты

```lua
function bit_converter.int16_to_bytes(integer: int) -> table
```
Конвертирует знаковое 2-х битное целое число в байты

```lua
function bit_converter.int32_to_bytes(integer: int) -> table
```
Конвертирует знаковое 4-х битное целое число в байты

```lua
function bit_converter.int64_to_bytes(integer: int) -> table
```
Конвертирует знаковое 8-и битное целое число в байты

```lua
function bit_converter.bytes_to_string(table: bytes) -> string
```
Конвертирует массив байтов в строку

```lua
function bit_converter.byte_to_bool(integer: byte) -> boolean
```
Конвертирует байт в логическое булевое значение

```lua
function bit_converter.bytes_to_single(table: bytes) -> number№
```
Конвертирует массив байтов в плавающее число одинарной точности

```lua
function bit_converter.bytes_to_double(table: bytes) -> number
```
Конвертирует массив байтов в плавающее число двойной точности

```lua
function bit_converter.bytes_to_uint16(table: bytes) -> integer
```
Конвертирует массив байтов в 2-х битное беззнаковое число

```lua
function bit_converter.bytes_to_uint32(table: bytes) -> integer
```
Конвертирует массив байтов в 4-х битное беззнаковое число

```lua
function bit_converter.bytes_to_int16(table: bytes) -> integer
```
Конвертирует массив байтов в 2-х битное знаковое число

```lua
function bit_converter.bytes_to_int32(table: bytes) -> integer
```
Конвертирует массив байтов в 4-х битное знаковое число

```lua
function bit_converter.bytes_to_int64(table: bytes) -> integer
```
Конвертирует массив байтов в 8-х битное знаковое число