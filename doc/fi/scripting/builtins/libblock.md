# Kirjasto *block*

```lua
-- Palauttaa lohkon merkkijonotunnuksen sen numeerisen tunnuksen perusteella.
block.name(blockid: int) -> str

-- Palauttaa lohkon numeerisen tunnuksen ja ottaa argumentiksi merkkijonon
block.index(name: str) -> int

-- Palauttaa lohkomateriaalin tunnuksen.
block.material(blockid: int) -> str

-- Palauttaa käyttöliittymässä näkyvän lohkon nimen.
block.caption(blockid: int) -> str

-- Palauttaa lohkon numeerisen id:n määritetyissä koordinaateissa.
-- Jos määritetyissä koordinaateissa olevaa osaa ei ladata, palauttaa -1.
block.get(x: int, y: int, z: int) -> int

-- Palauttaa täydellisen tilan (kierto + segmentti + lisätiedot) kokonaislukuna
block.get_states(x: int, y: int, z: int) -> int

-- Asentaa lohkon, jolla on annettu numeerinen tunnus ja tila (0 on oletusarvo) annettuihin koordinaatteihin.
block.set(x: int, y: int, z: int, id: int, states: int)

-- Asentaa lohkon, jolla on annettu numeerinen tunnus ja tila (0 - oletusarvoisesti) annettuihin koordinaatteihin
-- pelaajan puolesta kutsumalla on_placed-tapahtumaa.
-- Playerid on valinnainen
block.place(x: int, y: int, z: int, id: int, states: int, [optional] playerid: int)

-- Katkaisee lohkon annetuista koordinaateista pelaajan näkökulmasta ja laukaisee on_broken -tapahtuman.
-- Playerid on valinnainen
block.destruct(x: int, y: int, z: int, playerid: int)

-- Kerää koko tilan kokonaislukuna
block.compose_state(state: {rotation: int, segment: int, userbits: int}) -> int

-- Jäsentää täydellisen tilan: kierto, segmentti, käyttäjäbitit
block.decompose_state(state: int) -> {int, int, int}
```

> [!WARNING]
> `block.set` ei laukaise on_placed-tapahtumaa.

```lua
-- Tarkistaa, onko lause määritettyjen koordinaattien kohdalla valmis
block.is_solid_at(x: int, y: int, z: int) -> bool

-- Tarkistaa, onko mahdollista sijoittaa lause annettuihin koordinaatteihin
-- (esimerkkejä: ilma, ruoho, kukat, vesi)
block.is_replaceable_at(x: int, y: int, z: int) -> bool

-- Palauttaa ladatussa sisällössä käytettävissä olevien lohkotunnusten määrän
block.defs_count() -> int

-- Palauttaa *picking-item*-ominaisuudessa määritetyn tuotteen numeerisen tunnuksen.
block.get_picking_item(id: int) -> int
```

### Raycast

```lua
block.raycast(start: vec3, dir: vec3, max_distance: number, [valinnainen] dest: table, [valinnainen] filter: table) -> {
    block: int, -- lohkon id
    endpoint: vec3, -- palkin kosketuspiste
    iendpoint: vec3, -- palkin koskettaman kappaleen sijainti
    length: number, -- säteen pituus
    normal: vec3, -- Säteen koskettaman pinnan normaalivektori
} tai nil
```

Heittää säteen aloituspisteestä ohjaajan suuntaan. Max_distance määrittää säteen enimmäispituuden.

`Filter`-argumentin avulla voit määrittää, mitkä lohkot ovat "läpinäkyviä" säteelle. Huomaa: {"base:glass","base:water"}.
Käyttääksesi `dest`-argumenttia sinun on täytettävä se jollakin (nolla on mahdollista), tämä tehdään taaksepäin yhteensopivuuden vuoksi

Funktio palauttaa taulukon, jossa on tulokset tai nolla, jos säde ei kosketa lohkoa.

Tulos käyttää kohdetaulukkoa uuden luomisen sijaan, jos valinnainen argumentti on määritetty.

## Kierto

Seuraavia toimintoja käytetään huomioimaan lohkon kierto käytettäessä vierekkäisiä lohkoja tai muita tarkoituksia, joissa lohkon suunta on kriittinen.


```lua
-- Palauttaa lohkon kokonaisluvun yksikkövektorin X määritetyissä koordinaateissa, ottaen huomioon sen rotaatio (kolme kokonaislukua).
-- Jos kiertoa ei ole, palauttaa 1, 0, 0
block.get_X(x: int, y: int, z: int) -> int, int, int

-- Sama, mutta Y-akselille (oletus 0, 1, 0)
block.get_Y(x: int, y: int, z: int) -> int, int, int

-- Sama, mutta Z-akselille (oletus 0, 0, 1)
block.get_Z(x: int, y: int, z: int) -> int, int, int

-- Palauttaa lohkon kiertoindeksin sen kiertoprofiilissa (enintään 7).
block.get_rotation(x: int, y: int, z: int) -> int

-- Asettaa lohkon kierron indeksin mukaan sen kiertoprofiilissa.
block.set_rotation(x: int, y: int, z: int, rotation: int)

-- Palauttaa kiertoprofiilin nimen (ei mitään/ruutu/putki)
block.get_rotation_profile(id: int) -> str
```

## Laajennetut lohkot

Laajennetut lohkot ovat niitä, joiden koko on suurempi kuin 1x1x1

```lua
-- Tarkistaa, onko lohko pidennetty.
block.is_extended(id: int) -> bool

-- Palauttaa lohkon koon.
block.get_size(id: int) -> int, int, int

-- Tarkistaa, onko lohko laajennetun lohkon muu kuin pääsegmentti.
block.is_segment(x: int, y: int, z: int) -> bool

-- Palauttaa laajennetun lohkon pääsegmentin sijainnin tai alkuperäisen sijainnin,
-- jos lohkoa ei ole pidennetty.
block.seek_origin(x: int, y: int, z: int) -> int, int, int
```

## Mukautetut bitit

Osa `voxel.states`-kentästä, joka on varattu käytettäväksi skripteissä ja joka tallentaa lisätietoja vokselista, kuten lohkon kierto. Tällä hetkellä varattu osuus on 8 bittiä.

```lua
-- Palauttaa valitun bittimäärän määritetystä siirtymästä etumerkittömänä kokonaislukuna
block.get_user_bits(x: int, y: int, z: int, offset: int, bits: int) -> int

-- Kirjoittaa määritetyn määrän arvobittejä käyttäjän biteille valitulla siirtymällä
block.set_user_bits(x: int, y: int, z: int, offset: int, bits: int, value: int) -> int
```


## Fysiikka

Tietoja fysiikkamoottorin käyttämistä lohkoominaisuuksista.

```lua
-- Palauttaa kahden vektorin taulukon (3 luvun taulukot):
-- 1. Hitboxin vähimmäispiste
-- 2. Hitboxin koko
-- rotation_index - lohkon kiertoindeksi
block.get_hitbox(id: int, rotation_index: int) -> {vec3, vec3}
```

## Malli

Tietoa lohkomallista.

```lua
-- palauttaa lohkomallin tyypin (block/aabb/custom/...)
block.get_model(id: int) -> str

-- palauttaa 6 pintakuvion joukon, jotka on määritetty lohkon sivuille
block.get_textures(id: int) -> таблица строк
```

## Tietokentät

```lua
-- kirjoittaa arvon määritettyyn lohkokenttään
-- * antaa poikkeuksen, kun tyypit eivät ole yhteensopivia
-- * heittää poikkeuksen, kun taulukko on rajojen ulkopuolella
-- * ei tee mitään, jos lohkolla ei ole kenttää
block.set_field(
    x: int, y: int, z: int, 
    name: str,
    value: bool|int|number|string, 
    [valinnainen] index: int = 0
)

-- palauttaa lohkokenttään kirjoitetun arvon
-- * palauttaa nollan, jos:
-- 1. kenttää ei ole olemassa
-- 2. lohkon mihinkään kenttään ei tehty merkintöjä
-- * heittää poikkeuksen, kun taulukko on rajojen ulkopuolella
block.get_field(
    x: int, y: int, z: int, 
    name: str, 
    [valinnainen] index: int = 0
) -> tallennettu arvo tai nil
```
