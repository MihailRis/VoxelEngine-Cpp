# Module core:data_buffer

## Data buffer
### Stores an array of bytes and allows you to easily get or add different values

```lua
function data_buffer(bytes)
```
Creates a new data_buffer instance (the bytes parameter is optional)

```lua
function data_buffer:put_byte(integer: byte)
```
Writes a byte to the buffer

```lua
function data_buffer:put_bytes(table: bytes)
```
Writes bytes to the buffer

```lua
function data_buffer:put_string(string: str)
```
Converts a string to bytes and writes them to the buffer

```lua
function data_buffer:put_bool(boolean: bool)
```
Converts a boolean value to a byte and writes it to the buffer

```lua
function data_buffer:put_single(number: single)
```
Converts a single precision float to bytes and writes them to the buffer

```lua
function data_buffer:put_double(number: double)
```
Converts a double precision float to bytes and writes them to the buffer

```lua
function data_buffer:put_uint16(integer: int)
```
Converts an unsigned 2-bytes number to bytes and writes them to the buffer

```lua
function data_buffer:put_uint32(integer: int)
```
Converts an unsigned 4-bytes number to bytes and writes them to the buffer

```lua
function data_buffer:put_int16(integer: int)
```
Converts a signed 2-bytes number into bytes and writes them to the buffer

```lua
function data_buffer:put_int32(integer: int)
```
Converts a signed 4-bytes number into bytes and writes them to the buffer

```lua
function data_buffer:put_int64(integer: int)
```
Converts a signed 8-bytes number to bytes and writes them to the buffer

```lua
function data_buffer:put_number(number: num)
```
Converts any number into bytes and writes them to the buffer;

The first byte is the value type:
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
Returns the next byte from the buffer

```lua
function data_buffer:get_bytes(n) -> table
```
Returns the next n bytes, if n is nil or not specified, then an array of all bytes is returned

```lua
function data_buffer:get_string() -> string
```
Reads the next line from the buffer

```lua
function data_buffer:get_bool() -> boolean
```
Reads the next Boolean from the buffer

```lua
function data_buffer:get_single() -> number
```
Reads the next single precision floating number from the buffer

```lua
function data_buffer:get_double() -> number
```
Reads the next double precision floating number from the buffer

```lua
function data_buffer:get_uint16() -> integer
```
Reads the next 2-bytes unsigned integer from the buffer

```lua
function data_buffer:get_uint32() -> integer
```
Reads the next 4-bytes unsigned integer from the buffer

```lua
function data_buffer:get_int16() -> integer
```
Reads the next 2-bytes signed integer from the buffer

```lua
function data_buffer:get_int32() -> integer
```
Reads the next 4-bytes signed integer from the buffer

```lua
function data_buffer:get_int64() -> integer
```
Reads the next 8-bytes signed integer from the buffer

```lua
function data_buffer:get_number() -> number
```
Reads the next number (see data_buffer:put_number)

```lua
function data_buffer:size() -> integer
```
Returns the buffer size

```lua
function data_buffer:set_position(integer: pos)
```
Sets the current position in the buffer

```lua
function data_buffer:set_bytes(table: bytes)
```
Sets bytes into the buffer
