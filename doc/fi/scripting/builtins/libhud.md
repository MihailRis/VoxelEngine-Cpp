# Kirjasto hud

```lua
-- Avaa varaston.
hud.open_inventory()

-- Sulkee varaston.
hud.close_inventory()

-- Avaa varaston ja estää käyttöliittymän.
-- Jos lohkolla ei ole käyttöliittymän asettelua, tehdään poikkeus.
-- Palauttaa lohkon inventaariotunnuksen
-- (*"inventory-size"=0* luodaan virtuaalinen varasto,
-- joka poistetaan sulkemisen jälkeen) ja käyttöliittymän asettelutunnus.
hud.open_block(x: int, y: int, z: int) -> int, str
```

> [!NOTE]
> Vain yksi lohko voi olla auki kerrallaan

```lua
-- Näyttää elementin peittokuvatilassa.
-- Näyttää myös pelaajan inventaarion, jos playerinv on **true**.
hud.show_overlay(layoutid: str, playerinv: bool)

-- Lisää näyttöön pysyvän elementin. Elementtiä ei poisteta, kun
-- varaston sulkeminen. Jotta varjostus ei mene päällekkäin tilassa
-- varaston on asetettava elementin z-indeksi pienemmäksi kuin -1.
-- Varastotunnisteen tapauksessa paikat linkitetään pelaajan varastoon.
hud.open_permanent(layoutid: str)

-- Poistaa elementin näytöltä.
hud.close(layoutid: str)

-- Antaa avoimen lohkon inventaariotunnuksen tai 0.
hud.get_block_inventory() -> int

-- Antaa sen soittimen tunnuksen, johon käyttöliittymä on sidottu.
hud.get_player() -> int

-- Avaa taukovalikon.
hud.pause()

-- Sulkee taukovalikon.
hud.resume()

-- Palauttaa tosi, jos taukovalikko on auki.
hud.is_paused() -> bool

-- Palauttaa tosi, jos mainosjakauma tai peittokuva on auki.
hud.is_inventory_open() -> bool
```
