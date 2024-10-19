# Moduli core:data_buffer

## Datapuskuri
### Tallentaa joukon tavuja ja voit helposti hankkia tai lisätä erilaisia ​​arvoja

```lua
function data_buffer(bytes)
```
Luo uuden data_buffer-instanssin (tavut-parametri on valinnainen)

```lua
function data_buffer:put_byte(integer: byte)
```
Kirjoittaa tavun puskuriin

```lua
function data_buffer:put_bytes(table: bytes)
```
Kirjoittaa tavuja puskuriin

```lua
function data_buffer:put_string(string: str)
```
Muuntaa merkkijonon tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_bool(boolean: bool)
```
Muuntaa loogisen arvon tavuksi ja kirjoittaa sen puskuriin

```lua
function data_buffer:put_single(number: single)
```
Muuntaa yhden tarkkuuden floatin tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_double(number: double)
```
Muuntaa kaksinkertaisen tarkkuuden floatin tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_uint16(integer: int)
```
Muuntaa etumerkittömän 2-tavuisen luvun tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_uint32(integer: int)
```
Muuntaa etumerkittömän 4-tavuisen luvun tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_int16(integer: int)
```
Muuntaa allekirjoitetun 2-tavuisen luvun tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_int32(integer: int)
```
Muuntaa allekirjoitetun 4-tavuisen luvun tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_int64(integer: int)
```
Muuntaa allekirjoitetun 8-tavuisen luvun tavuiksi ja kirjoittaa ne puskuriin

```lua
function data_buffer:put_number(number: num)
```
Muuntaa minkä tahansa luvun tavuiksi ja kirjoittaa ne puskuriin;

Ensimmäinen tavu on arvotyyppi:
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
Palauttaa seuraavan tavun puskurista

```lua
function data_buffer:get_bytes(n) -> table
```
Palauttaa seuraavat n tavua, jos n on nolla tai sitä ei ole määritetty, palautetaan kaikkien tavujen joukko

```lua
function data_buffer:get_string() -> string
```
Lukee seuraavan rivin puskurista

```lua
function data_buffer:get_bool() -> boolean
```
Lukee seuraavan Boolen puskurista

```lua
function data_buffer:get_single() -> number
```
Lukee seuraavan yksittäisen tarkkuuskelluvan luvun puskurista

```lua
function data_buffer:get_double() -> number
```
Lukee seuraavan kaksinkertaisen tarkkuuden kelluvan luvun puskurista

```lua
function data_buffer:get_uint16() -> integer
```
Lukee puskurista seuraavan 2-tavun etumerkittömän kokonaisluvun

```lua
function data_buffer:get_uint32() -> integer
```
Lukee puskurista seuraavan 4-tavun etumerkittömän kokonaisluvun

```lua
function data_buffer:get_int16() -> integer
```
Lukee puskurista seuraavan 2-tavun etumerkillisen kokonaisluvun

```lua
function data_buffer:get_int32() -> integer
```
Lukee puskurista seuraavan 4-tavun etumerkillisen kokonaisluvun

```lua
function data_buffer:get_int64() -> integer
```
Lukee puskurista seuraavan 8-tavun etumerkillisen kokonaisluvun

```lua
function data_buffer:get_number() -> number
```
Lukee seuraavan numeron (katso data_buffer:put_number)

```lua
function data_buffer:size() -> integer
```
Palauttaa puskurin koon

```lua
function data_buffer:set_position(integer: pos)
```
Asettaa nykyisen sijainnin puskurissa

```lua
function data_buffer:set_bytes(table: bytes)
```
Asettaa tavut puskuriin
