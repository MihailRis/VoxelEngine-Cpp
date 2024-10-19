# Kirjasto *cameras*

Kirjasto on suunniteltu toimimaan kameroiden kanssa.

## Perustoiminnot

```lua
cameras.get(name: str) -> table
-- tai
cameras.get(index: int) -> table
```

Palauttaa kameran nimen tai indeksin perusteella.

## Kameramenetelmät

```lua
-- Ota kamera
local cam = cameras.get("pakkaus: kameran nimi")

-- palauttaa kameran indexin
cam:get_index() -> int

-- palauttaa kameran nimen
cam:get_name() -> str

-- palauttaa kameran asennon
cam:get_pos() -> vec3
-- asettaa kameran asennon
cam:set_pos(pos: vec3)

-- palauttaa kameran kierron
cam:get_rot() -> mat4
-- asettaa kameran kierron
cam:set_rot(rot: mat4)

-- palauttaa kameran zoomausarvon
cam:get_zoom() -> number
-- asettaa kameran zoomausarvon
cam:set_zoom(zoom: number)

-- palauttaa kameran näkökentän kulman Y:ssä (asteina)
cam:get_fov() -> number
-- asettaa kameran kuvakulman Y:ssä (asteina)
cam:set_fov(fov: number)

-- palauttaa tosi, jos Y-akselia käännetään
cam:is_flipped() -> bool
-- heijastaa Y-akselia, kun tosi
cam:set_flipped(flipped: bool)

-- tarkistaa, onko perspektiivitila käytössä
cam:is_perspective() -> bool
-- ottaa perspektiivitilan käyttöön/pois päältä
cam:set_perspective(perspective: bool)

-- palauttaa kameran suuntavektorin
cam:get_front() -> vec3
-- palauttaa suuntavektorin oikealle
cam:get_right() -> vec3
-- palauttaa suuntavektorin ylös
cam:get_up() -> vec3

-- ohjaa kameran tiettyyn pisteeseen
cam:look_at(point: vec3)

-- ohjaa kameran tiettyyn pisteeseen interpolointikertoimella
cam:look_at(point: vec3, t: number)
```

Kameran vaihtaminen on mahdollista player.set_camera-toiminnon kautta.
