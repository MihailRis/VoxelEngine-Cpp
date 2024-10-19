# Kirjasto *item*

```lua
-- Palauttaa kohteen merkkijonotunnuksen sen numeerisen tunnuksen perusteella (kuten block.name)
item.name(itemid: int) -> str

-- Palauttaa kohteen numeerisen tunnuksen merkkijonotunnuksella (kuten block_index)
item.index(name: str) -> int

-- Palauttaa kohteen nimen sellaisena kuin se näkyy käyttöliittymässä.
item.caption(blockid: int) -> str

-- Palauttaa kohteen enimmäispinon koon.
item.stack_size(itemid: int) -> int

-- Palauttaa käytettävissä olevien kohteiden kokonaismäärän (mukaan lukien luodut)
item.defs_count() -> int

-- Palauttaa kohdekuvakkeen nimen käytettäväksi kuvaelementin src-ominaisuudessa
item.icon(itemid: int) -> str
```


