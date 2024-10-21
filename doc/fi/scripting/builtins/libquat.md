# Kirjasto *quat*

Kirjasto quaternionien kanssa työskentelemiseen.

## Quaternion matriisista - *mat4.from_quat(...)*

```lua
-- luo kvaternionin rotaatiomatriisin perusteella
quat.from_mat4(m: matrix)

-- kirjoittaa kvaternionin rotaatiomatriisista arvoon dst
quat.from_mat4(m: matrix, dst: quat)
```

## Pallomainen lineaarinen interpolointi - *quat.slerp(...)*

Interpolointi suoritetaan aina lyhyttä polkua pitkin ja pyöriminen tapahtuu vakionopeudella.

```lua
-- luo kvaternionin interpolaationa a:n ja b:n välille,
-- missä t on interpolointikerroin
quat.slerp(a: quat, b: quat, t: number)

-- kirjoittaa kvaternionin interpolaationa a:n ja b:n välillä dst:ssä,
-- missä t on interpolointikerroin
quat.slerp(a: quat, b: quat, t: number, dst: quat)
```

## Käännös merkkijonoon - *quat.tostring(...)*

```lua
-- palauttaa merkkijonon, joka edustaa kvaternionin sisältöä
quat.tostring(q: quat)
```
