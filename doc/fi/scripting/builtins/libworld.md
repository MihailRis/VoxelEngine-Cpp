# Kirjasto *world*

```lua
-- Palauttaa tietoa maailmoista.
world.get_list() -> массив таблиц {
    -- maailman nimi
    name: str,
    -- esikatselu (automaattisesti ladattu tekstuuri)
    icon: str
}

-- Palauttaa nykyisen peliajan 0.0:sta 1.0:aan, jossa 0.0 ja 1.0 ovat keskiyötä ja 0.5 keskipäivää.
world.get_day_time() -> number

-- Asettaa määritetyn peliajan.
world.set_day_time(time: number)

-- Asettaa määritetyn nopeuden kellonajan vaihtamiseen.
world.set_day_time_speed(value: number)

-- Palauttaa nopeuden, jolla kellonaika vaihtuu.
world.get_day_time_speed() -> number

-- Palauttaa maailmassa kuluneen kokonaisajan.
world.get_total_time() -> number

-- Palauttaa maailman viljan.
world.get_seed() -> int

-- Tarkistaa maailman olemassaolon nimellä.
world.exists() -> bool

-- Tarkistaa, onko nykyinen aika päivä. Klo 0.333 (8.00) - 0.833.
world.is_day() -> bool

-- Tarkistaa, onko nykyinen aika yö. Klo 0.833 (20.00) - 0.333.
world.is_night() -> bool
```
