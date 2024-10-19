# Kirjasto Vec*n*

*vecn* sisältää joukon toimintoja työskennelläksesi vektoreiden kanssa, joiden mitat ovat 2, 3 tai 4.
Useimmissa funktioissa on useita vaihtoehtoja argumenttiluetteloille (ylikuormitukset).

> [!WARNING]
>
> vecn, jossa n == vektorin dimensio (2, 3, 4), eli vec2, vec3, vec4
> 

## Tietotyypit

Tällä sivulla käytetään tyyppikäytäntöjä.
- vektori - kahden, kolmen tai neljän luvun joukko
- vec2 - kahden luvun joukko
- vec3 - kolmen luvun joukko
- vec4 - neljän luvun joukko

> [!WARNING]
>
> Tyyppimerkinnät ovat osa dokumentaatiota, eikä niitä määritetä kutsuttaessa.


## Operaatiot vektoreilla

#### Lisäys - *vecn.add(...)*

```lua
-- palauttaa vektorin lisäyksen tuloksen
vecn.add(a: vector, b: vector)

-- palauttaa vektorin ja skalaarin lisäämisen tuloksen
vecn.add(a: vector, b: number)

-- kirjoittaa tuloksen kahden vektorin lisäämisestä dst:hen
vecn.add(a: vector, b: vector, dst: vector)
```

#### Vähennyslasku - *vecn.sub(...)*

```lua
-- palauttaa vektorivähennystuloksen
vecn.sub(a: vector, b: vector)

-- palauttaa tuloksen, kun skalaari vähennetään vektorista
vecn.sub(a: vector, b: number)

-- kirjoittaa tuloksen kahden vektorin vähentämisestä dst:hen
vecn.sub(a: vector, b: vector, dst: vector)
```

#### Kertominen - *vecn.mul(...)*

```lua
-- palauttaa vektorin kertolaskutuloksen
vecn.mul(a: vector, b: vector)

-- palauttaa tuloksen kertomalla vektorin skalaarilla
vecn.mul(a: vector, b: number)
```

#### Inversio - *vecn.inv(...)*

```lua
-- palauttaa vektorin käännöstuloksen (vastakohta).
vecn.inverse(a: vector)

-- kirjoittaa käänteisen vektorin dst:hen
vecn.inverse(v: vector, dst: vector)
```

####  Division - *vecn.div(...)*

```lua
-- palauttaa vektorijaon tuloksen
vecn.div(a: vector, b: vector)

-- palauttaa vektorin skalaarilla jakamisen tuloksen
vecn.div(a: vector, b: number)

-- kirjoittaa tuloksen kahden vektorin jakamisesta arvoon dst
vecn.div(a: vector, b: vector, dst: vector)
```

#### Normalisointi - *vecn.norm(...)*

```lua
-- palauttaa normalisoidun vektorin
vecn.normalize(a: vector)

-- kirjoittaa normalisoidun vektorin dst:hen
vecn.normalize(v: vector, dst: vector)
```

#### Vektorin pituus - *vecn.len(...)*

```lua
-- palauttaa vektorin pituuden
vecn.length(a: vector)

```

#### Absoluuttinen arvo - *vecn.abs(...)*

```lua
-- palauttaa vektorin absoluuttisilla arvoilla
vecn.abs(a: vector)

-- kirjoittaa vektorin itseisarvon arvoon dst
vecn.abs(v: vector, dst: vector)
```

#### Pyöristys - *vecn.round(...)*

```lua
-- palauttaa vektorin pyöristetyillä arvoilla
vecn.round(a: vector)

-- kirjoittaa pyöristetyn vektorin muotoon dst
vecn.round(v: vector, dst: vector)
```

#### Tutkinto - *vecn.pow(...)*

```lua
-- palauttaa vektorin, jonka elementit on korotettu potenssiin
vecn.pow(a: vector, b: number)

-- kirjoittaa potenssiin korotetun vektorin arvoon dst
vecn.pow(v: vector, exponent: number, dst: vector)
```

#### Pistetuote - *vecn.dot(...)*
```lua
-- palauttaa vektorien skalaaritulon
vecn.dot(a: vector, b: vector)
```

#### Käännös merkkijonoon - *vecn.tostring(...)*
> [!WARNING]
> Palauttaa vain, jos sisältö on vektori
```lua
-- palauttaa merkkijonon, joka edustaa vektorin sisältöä
vecn.tostring(a: vector)
```

## Erityiset toiminnot

Vektorimittoihin liittyvät funktiot.

```lua
-- palauttaa satunnaisvektorin, jonka koordinaatit ovat tasaisesti jakautuneet tietyn säteen pallolle
vec3.spherical_rand(radius: number)

-- kirjoittaa satunnaisvektorin, jonka koordinaatit ovat tasaisesti jakautuneet tietyn säteen omaavalle pallolle dst:ssä
vec3.spherical_rand(radius: number, dst: vec3)

-- palauttaa vektorin v suuntakulman asteina [0, 360]
vec2.angle(v: vec2)

-- palauttaa vektorin {x, y} suuntakulman asteina [0, 360]
vec2.angle(x: number, y: number) 
```


## Esimerkki
```lua
-- luoda eri mittaisia ​​vektoreita
local v1_3d = {1, 2, 2}
local v2_3d = {10, 20, 40}
local v3_4d = {1, 2, 4, 1}
local v4_2d = {1, 0}
local scal = 6 -- tavallinen skalaari

-- vektorin lisäys
local result_add = vec3.add(v1_3d, v2_3d)
print("add: " .. vec3.tostring(result_add)) -- {11, 22, 42}

-- vektorinvähennys
local result_sub = vec3.sub(v2_3d, v1_3d)
print("sub: " .. vec3.tostring(result_sub)) -- {9, 18, 38}

-- vektorinkertaus
local result_mul = vec3.mul(v1_3d, v2_3d)
print("mul: " .. vec3.tostring(result_mul)) -- {10, 40, 80}

-- vektorin kertominen skalaarilla
local result_mul_scal = vec3.mul(v1_3d, scal)
print("mul_scal: " .. vec3.tostring(result_mul_scal)) -- {6, 12, 12}

-- vektorin normalisointi
local result_norm = vec3.normalize(v1_3d)
print("norm: " .. vec3.tostring(result_norm)) -- {0.333, 0.667, 0.667}

-- vektorin pituus
local result_len = vec3.length(v1_3d)
print("len: " .. result_len) -- 3

-- vektorin itseisarvo
local result_abs = vec3.abs(v1_3d)
print("abs: " .. vec3.tostring(result_abs)) -- {1, 2, 2}

-- vektorin pyöristys
local result_round = vec3.round(v1_3d)
print("round: " .. vec3.tostring(result_round)) -- {1, 2, 2}

-- vektori tutkinto
local result_pow = vec3.pow(v1_3d, 2)
print("pow: " .. vec3.tostring(result_pow)) -- {1, 4, 4}

-- vektorien skalaaritulo
local result_dot = vec3.dot(v1_3d, v2_3d)
print("dot: " .. result_dot) -- 250
```
