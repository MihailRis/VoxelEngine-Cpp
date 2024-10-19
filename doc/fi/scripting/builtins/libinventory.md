# Kirjasto *inventory*

Toimintojen kirjasto varaston kanssa työskentelyä varten.

```lua
-- Palauttaa tuotteen tunnuksen ja sen määrän. id = 0 (ydin:tyhjä) tarkoittaa, että paikka on tyhjä.
inventory.get(
    -- varastotunnus(id)
    invid: int,
    -- paikkaindeksi
    slot: int
) -> int, int

-- Asettaa paikan sisällön.
inventory.set(
    -- varastotunnus(id)
    invid: int,
    -- paikkaindeksi
    slot: int,
    -- tuotetunnus
    itemid: int,
    - tavaran määrä
    count: int
)

-- Palauttaa varaston koon (paikkojen lukumäärän).
-- Jos määritettyä varastoa ei ole, tekee poikkeuksen.
inventory.size(invid: int) -> int

-- Lisää tavaran varastoon.
-- Jos koko määrää ei ollut mahdollista sovittaa, palauttaa loput.
inventory.add(
    -- varastotunnus(id)
    invid: int, 
    -- tuotetunnus(id)
    itemid: int, 
    -- tavaran määrä
    count: int
) -> int

-- Funktio palauttaa lohkon inventaariotunnuksen(id).
-- Jos lohkolla ei voi olla varastoa, palauttaa 0.
inventory.get_block(x: int, y: int, z: int) -> int

-- Sitoo määritellyn varaston lohkoon.
inventory.bind_block(invid: int, x: int, y: int, z: int)

-- Irrottaa varaston lohkosta.
inventory.unbind_block(x: int, y: int, z: int)
```

> [!WARNING]
> Varasto, joka ei ole sidottu mihinkään lohkoon, poistetaan, kun se poistuu maailmasta.

```lua
-- Luo kopion inventaariosta ja palauttaa kopion tunnuksen.
-- Jos kopioitua varastoa ei ole, palauttaa 0.
inventory.clone(invid: int) -> int

-- Siirtää kohteen invA:n varaston paikasta A paikkaan B invB:n inventaariossa.
-- invA ja invB voivat osoittaa samaan varastoon.
-- SlotB valitaan automaattisesti, jos sitä ei ole erikseen määritetty.
inventory.move(invA: int, slotA: int, invB: int, slotB: int)
```


