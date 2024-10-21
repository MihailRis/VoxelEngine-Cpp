# Kirjasto *mat4*

*mat4* sisältää joukon funktioita 4x4-mittaisten muunnosmatriisien käsittelyä varten.

Useimmissa funktioissa on useita vaihtoehtoja argumenttiluetteloille (ylikuormitukset).

## Identiteettimatriisi - *mat4.idt(...)*

```lua
-- luo identiteettimatriisin
mat4.idt()

-- kirjoittaa identiteettimatriisin dst:hen
mat4.idt(dst: matrix)
```

## Matriisin determinantti - *mat4.determinant(...)*

```lua
-- laskee matriisin determinantin
mat4.determinant(m: matrix)
```

## Matriisi kvaternionista - *mat4.from_quat(...)*

```lua
-- luo rotaatiomatriisin kvaternionin mukaan
mat4.from_quat(quaternion: quat)

-- kirjoittaa kvaternionin rotaatiomatriisin arvoon dst
mat4.from_quat(quaternion: quat, dst: matrix)
```

## Matriisin kertolasku - *mat4.mul(...)*

```lua
-- palauttaa matriisin kertolaskutuloksen
mat4.mul(a: matrix, b: matrix)
-- kirjoittaa matriisin kertolaskutuloksen dst:hen
mat4.mul(a: matrix, b: matrix, dst: matrix)

-- palauttaa matriisin ja vektorin kertolaskutuloksen
mat4.mul(a: matrix, v: vector)
-- kirjoittaa matriisin ja vektorin kertolaskutuloksen dst:hen
mat4.mul(a: matrix, v: vector, dst: vector)
```

## Matriisin inversio - *mat4.inverse(...)*

```lua
-- palauttaa matriisin inversion tuloksen
mat4.inverse(m: matrix)
-- kirjoittaa matriisin inversion tuloksen muotoon dst
mat4.inverse(m: matrix, dst: matrix)
```

##  Matriisitransponointi - *mat4.transpose(...)*

```lua
-- palauttaa matriisitransponoinnin tuloksen
mat4.transpose(m: matrix)
-- kirjoittaa matriisitransponoinnin tuloksen arvoon dst
mat4.transpose(m: matrix, dst: matrix)
```

## Offset - *mat4.translate(...)*

```lua
-- luo siirtymämatriisin
mat4.translate(translation: vec3)
-- palauttaa matriisiin m siirtymän soveltamisen tuloksen
mat4.translate(m: matrix, translation: vec3)
-- kirjoittaa tuloksen siirron soveltamisesta matriisiin m ja dst
mat4.translate(m: matrix, translation: vec3, dst: matrix)
```
## Skaalaus - *mat4.scale(...)*

```lua
-- luo skaalausmatriisin
mat4.scale(scale: vec3)
-- palauttaa matriisin m skaalauksen tuloksen
mat4.scale(m: matrix, scale: vec3)
-- kirjoittaa tuloksen skaalauksen soveltamisesta matriisiin m - dst
mat4.scale(m: matrix, scale: vec3, dst: matrix)
```

## Kierto - *mat4.rotate(...)*

Pyörimiskulma ilmoitetaan asteina.

```lua
-- luo rotaatiomatriisin (kulma - kiertokulma) tiettyä akselia pitkin (akseli - yksikkövektori)
mat4.rotate(axis: vec3, angle: number)
-- palauttaa matriisiin m käytetyn rotaation tuloksen
mat4.rotate(m: matrix, axis: vec3, angle: number)
-- kirjoittaa tuloksen, kun käytetään rotaatiota matriisiin m - dst
mat4.rotate(m: matrix, axis: vec3, angle: number, dst: matrix)
```

## Hajoaminen - *mat4.decompose(...)*

Hajottaa muunnosmatriisin komponentteihinsa.

```lua
mat4.decompose(m: matrix)
-- palautustaulukko:
{
    scale=vec3,
    rotation=matrix,
    quaternion=quat,
    translation=vec3,
    skew=vec3,
    perspective=vec4 
} tai nil
```

## Pisteiden seuranta *mat4.look_at(...)*

```lua
-- luo näkymämatriisin "eye" -pisteestä "keskipisteeseen", jossa "center"-vektori määrittelee up.
mat4.look_at(eye: vec3, center: vec3, up: vec3)
-- kirjoittaa näkymämatriisin dst:hen
mat4.look_at(eye: vec3, center: vec3, up: vec3, dst: matrix)
```

## Käännös merkkijonoon - *mat4.tostring(...)*

```lua
-- palauttaa matriisin sisältöä edustavan merkkijonon
mat4.tostring(m: matrix)
-- palauttaa matriisin sisältöä edustavan merkkijonon, monirivinen, jos monirivi = true
mat4.tostring(m: matrix, multiline: bool)
```
