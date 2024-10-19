# Moduli core:bit_converter

## Arvojen muuntaminen tavuiksi ja takaisin

```lua
function bit_converter.string_to_bytes(string: str) -> table
```
Muuntaa merkkijonon tavuiksi

```lua
function bit_converter.bool_to_byte(boolean: bool) -> integer
```
Muuntaa loogisen loogisen arvon tavuksi

```lua
function bit_converter.single_to_bytes(number: single) -> table
```
Muuntaa yhden tarkan kelluvan arvon tavuiksi

```lua
function bit_converter.double_to_bytes(number: double) -> table
```
Muuntaa kaksinkertaisen tarkkuuden float-arvon tavuiksi

```lua
function bit_converter.uint16_to_bytes(integer: int) -> table
```
Muuntaa etumerkittömän 2-tavuisen kokonaisluvun tavuiksi

```lua
function bit_converter.uint32_to_bytes(integer: int) -> table
```
Muuntaa etumerkittömän 4-tavun kokonaisluvun tavuiksi

```lua
function bit_converter.int16_to_bytes(integer: int) -> table
```
Muuntaa etumerkityn 2-tavuisen kokonaisluvun tavuiksi

```lua
function bit_converter.int32_to_bytes(integer: int) -> table
```
Muuntaa etumerkityn 4-tavun kokonaisluvun tavuiksi

```lua
function bit_converter.int64_to_bytes(integer: int) -> table
```
Muuntaa etumerkityn 8-tavun kokonaisluvun tavuiksi

```lua
function bit_converter.bytes_to_string(table: bytes) -> string
```
Muuntaa tavutaulukon merkkijonoksi

```lua
function bit_converter.byte_to_bool(integer: byte) -> boolean
```
Muuntaa tavun boolen arvoksi

```lua
function bit_converter.bytes_to_single(table: bytes) -> number№
```
Muuntaa tavutaulukon yhden tarkkuuden kelluvaksi luvuksi

```lua
function bit_converter.bytes_to_double(table: bytes) -> number
```
Muuntaa tavutaulukon kaksinkertaisen tarkkuuden floatiksi

```lua
function bit_converter.bytes_to_uint16(table: bytes) -> integer
```
Muuntaa tavutaulukon 2-tavuiseksi etumerkittömäksi luvuksi

```lua
function bit_converter.bytes_to_uint32(table: bytes) -> integer
```
Muuntaa tavutaulukon 4-tavuiseksi etumerkittömäksi luvuksi

```lua
function bit_converter.bytes_to_int16(table: bytes) -> integer
```
Muuntaa tavutaulukon 2-tavuiseksi merkityksi numeroksi

```lua
function bit_converter.bytes_to_int32(table: bytes) -> integer
```
Muuntaa tavutaulukon 4-tavuiseksi merkityksi numeroksi

```lua
function bit_converter.bytes_to_int64(table: bytes) -> integer
```
Muuntaa tavutaulukon 8-tavuiseksi merkityksi numeroksi
