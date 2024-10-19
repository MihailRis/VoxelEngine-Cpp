# Kirjasto *entities*

Kirjasto on suunniteltu toimimaan kokonaisuusrekisterin kanssa.

```lua
-- Palauttaa kokonaisuuden yksilöllisen tunnisteen mukaan
-- Palautettu taulukko on sama, joka on saatavilla entiteettikomponenteissa.
entities.get(uid: int) -> table

-- Luo määritetyn kokonaisuuden.
-- args - komponenttiparametritaulukoiden taulukko (ARGS-muuttuja)
-- args on valinnainen
-- args rakenne:
-- {etuliite_nimi={...}, ...}
-- etuliite - paketin tunnus
-- nimi - komponentin nimi
-- etuliite ja komponentin nimi erotetaan kahdella alaviivalla
entities.spawn(name: str, pos: vec3, [optional] args: table)

-- Tarkistaa entiteetin olemassaolon yksilöllisen tunnisteen avulla.
entities.exists(uid: int) -> bool

-- Palauttaa entiteetin määritelmän indeksin UID:n mukaan
entities.get_def(uid: int) -> int

-- Palauttaa entiteetin määritelmän nimen indeksillä (merkkijonotunnus).
entities.def_name(id: int) -> str

-- Palauttaa entiteetin määritelmän indeksin nimen (numeerinen ID) mukaan.
entities.def_index(name: str) -> int

-- Palauttaa käytettävissä olevien entiteettimääritelmien määrän
entities.defs_count() -> int

-- Palauttaa taulukon kaikista ladatuista kokonaisuuksista
entities.get_all() -> table

-- Palauttaa ladattujen entiteettien taulukon hyväksyttyjen UID-luetteloiden perusteella
entities.get_all(uids: array<int>) -> table

-- Palauttaa luettelon kokonaisuuksien UID-tunnuksista, jotka kuuluvat suorakaiteen muotoiseen alueeseen
-- pos - pienin pintakulma
-- koko - alueen koko
entities.get_all_in_box(pos: vec3, size: vec3) -> array<int>

-- Palauttaa luettelon säteellä olevien entiteettien UID:istä
-- keskus - alueen keskus
-- säde - alueen säde
entities.get_all_in_radius(center: vec3, radius: number) -> array<int>
```

```lua
entities.raycast(start: vec3, dir: vec3, max_distance: number,
                 ignore: int, [optional] destination: table, [optional] filter: table) -> table tai nil
```

Funktio on [block.raycast](libblock.md#raycast) laajennettu versio. Palauttaa taulukon tuloksineen, jos säde koskettaa lohkoa tai kokonaisuutta.

Vastaavasti tämä vaikuttaa *entity*- ja *block*-kenttien olemassaoloon.
