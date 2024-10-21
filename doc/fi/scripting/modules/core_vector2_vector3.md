# Moduli core:vector2, core:vector3

## Vector2
### Operaatiot vektoreille


```lua
function vector2:round(decimals: number) -> round[vec2]
```
Pyöristysvektorikomponentit

```lua
function vector2:len() -> number
```
Vektorin pituus

```lua
function vector2:norm() -> number
```
Vektorin normalisointi

```lua
function vector2:abtw(vector: vec2) -> number
```
Kahden vektorin välinen kulma radiaaneina


```lua
function vector2:proj(vector: vec2) -> vec2
```
Vektoriprojektio

```lua
function vector2:dot(vector: vec2) -> number
```
Vektori (sisäinen) tuote

```lua
function vector2:lerp(
                     -- Kohdevektori
                     b: vec2, 
                     --Interpolointikerroin (0-1)
                     t: number
                     ) -> vec2
```
Lineaarinen vektorin interpolointi

```lua
function vector2:dist(vector: vec2) -> number
```
Kahden vektorin välinen etäisyys

```lua
function vector2:cross(vector: vec2) -> number
```
Vektori (ulkoinen) tuote

```lua
function vector2:rot(
                    -- vektorin kiertokulma tietyssä kulmassa (radiaaneina)
                    angle: number->rad, 
                    --Vektorin kierto akseleiden suhteen ("x", "y", "z")
                    axis: str, 
                    -- Jos totta, kiertokulma muunnetaan automaattisesti asteina radiaaneiksi
                    convert2deg: bool
                    ) -> vec2
```
Kierrä vektoria

### Operaatiot vektoreilla

```lua
    local vec2 = require("core:vector2")

    local v1 = vec2(5, 10)
    local v2 = vec2(10, 15)
    
    -- vec2 .. vec2
    sum_vectors = v1 + v2 -- (15, 25)
    sub_vectors = v1 - v2 -- (-5, -5)
    mul_vectors = v1 * v2 -- (50, 150)
    div_vectors = v1 / v2 -- (0.5, 0.66667.)
    pow_vectors = v1 ^ v2 -- (9765625, 1e+15)

    --vec2 .. scalar
    sum_vec2_scalar = v1 + 10 -- (15, 25)
    sub_vec2_scalar = v1 - 12 -- (-7, -2)
    mul_vec2_scalar = v1 * 20 -- (100, 200)
    div_vec2_scalar = v1 / 1 -- (5, 10)
    pow_vec2_scalar= v1 ^ 2 -- (25, 100)
```



## Vector3
### Operaatiot vektoreille

```lua
function vector3:round(decimals: number) -> round[vec3]
```
Pyöristysvektorikomponentit
```lua
function vector3:len() -> number
```
Vektorin pituus

```lua
function vector3:norm() -> number
```
Vektorin normalisointi

```lua
function vector3:abtw(vector: vec3) -> number
```
Kahden vektorin välinen kulma radiaaneina

```lua
function vector3:isParallel(vector: vec3) -> bool
```
Vektorin rinnakkaisuus toiseen vektoriin


```lua
function vector3:proj(vector: vec3) -> vec3
```
Vektoriprojektio
...

```lua
function vector3:dot(vector: vec3) -> number
```
Vektori (sisäinen) tuote

```lua
function vector3:lerp(
                     -- Kohdevektori
                     b: vec3, 
                     --Interpolointikerroin (0-1)
                     t: number
                     ) -> vec3
```
Lineaarinen vektorin interpolointi

```lua
function vector3:dist(vector: vec3) -> number
```
Kahden vektorin välinen etäisyys

```lua
function vector3:dist2line(point1: vec3, point2: vec3) -> number
```
Etäisyys linjaan

```lua
function vector3:cross(vector: vec3) -> number
```
Vektori (ulkoinen) tuote

```lua
function vector3:rot(
                    -- vektorin kiertokulma tietyssä kulmassa (radiaaneina)
                    angle: number->rad, 
                    --Vektorin kierto akseleiden suhteen ("x", "y", "z")
                    axis: str, 
                    -- Jos totta, kiertokulma muunnetaan automaattisesti asteina radiaaneiksi
                    convert2deg: bool
                    ) -> vec2
```
Kierrä vektoria

### Operaatiot vektoreilla

```lua
    
local vec3 = require("core:vector3")

local v1 = vec3(1, 2, 3)
local v2 = vec3(4, 5, 6)

-- vec3.. vec3
local sum_vectors = v1 + v2 -- (5, 7, 9)
local sub_vectors = v1 - v2 -- (-3, -3, -3)
local mul_vectors = v1 * v2 -- (4, 10, 18)
local div_vectors = v1 / v2 -- (0.25, 0.4, 0.5)
local pow_vectors = v1 ^ v2 -- (1, 32, 216)

-- vec3.. scalar
local scalar_vector = v1 * 2 -- (2, 4, 6)
local scalar_number = 2 * v1 -- (2, 4, 6)
-- ..etc

```

