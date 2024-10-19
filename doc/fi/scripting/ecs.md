# Kokonaisuudet ja komponentit

## Essence

Entiteettiobjekti on saatavilla komponenteissa globaalina muuttujana **entity**.

```lua
-- Poistaa entiteetin (entiteetti voi jatkaa olemassaoloaan kehyksen loppuun asti, mutta sitä ei näytetä kyseisessä kehyksessä)
entity:despawn()

-- Palauttaa entiteetin määritelmäindeksin (numeerinen tunnus)
entity:def_index() -> int

-- Palauttaa entiteetin määritelmän nimen (merkkijonotunnus)
entity:def_name() -> str

-- Palauttaa entiteettirungon nimen
entity:get_skeleton() -> str
-- Korvaa entiteettirungon
entity:set_skeleton(name: str)

-- Palauttaa yksilöllisen entiteetin tunnisteen
entity:get_uid() -> int

-- Palauttaa komponentin nimellä
entity:get_component(name: str) -> komponenttin tai nil
-- Tarkistaa osan olemassaolon nimen perusteella
entity:has_component(name: str) -> bool
```

## Sisäänrakennetut komponentit

### Muuttaa

Komponentti vastaa kokonaisuuden sijainnista, mittakaavasta ja pyörimisestä.

```lua
-- Vähentäminen
local tsf = entity.transform

-- Palauttaa kokonaisuuden sijainnin
tsf:get_pos() -> vec3
-- Asettaa kokonaisuuden sijainnin
tsf:set_pos(pos: vec3)

-- Palauttaa kokonaisuuden mittakaavan
tsf:get_size() -> vec3
-- Asettaa kokonaisuuden mittakaavan
tsf:set_size(size: vec3)

-- Palauttaa entiteetin kierron
tsf:get_rot() -> mat4
-- Asettaa kokonaisuuden kierron
tsf:set_rot(rotation: mat4)
```

### Rigidbody

Komponentti vastaa kokonaisuuden fyysisestä kehosta.

```lua
-- Vähentäminen
local body = entity.rigidbody

-- Tarkistaa, onko kehon fysiikan laskenta käytössä
body:is_enabled() -> bool
-- Ottaa käyttöön/poistaa käytöstä kehon fysiikan laskennan
body:set_enabled(enabled: bool)

-- Palauttaa lineaarisen nopeuden
body:get_vel() -> vec3
-- Asettaa lineaarisen nopeuden
body:set_vel(vel: vec3)

-- Palauttaa osumalaatikon koon
body:get_size() -> vec3
-- Asettaa osumalaatikon koon
body:set_size(size: vec3)

-- Palauttaa painovoiman kertoimen
body:get_gravity_scale() -> vec3
-- Asettaa painovoiman kertoimen
body:set_gravity_scale(scale: vec3)

-- Palauttaa lineaarisen nopeuden vaimenemiskertoimen (käytetään simuloimaan ilmanvastusta ja kitkaa)
body:get_linear_damping() -> number
-- Asettaa lineaarisen nopeuden vaimennuksen kertoimen
body:set_linear_damping(value: number)

-- Tarkistaa, onko pystysuora nopeuden vaimennus käytössä
body:is_vdamping() -> bool
-- Ottaa käyttöön/poistaa käytöstä pystynopeuden vaimennuksen
body:set_vdamping(enabled: bool)

-- Tarkistaa, onko kokonaisuus maassa (maadoitettu)
body:is_grounded() -> bool

-- Tarkistaa, onko entiteetti "harhaanjohtavassa" tilassa (ei voi pudota lohkoista)
body:is_crouching() -> bool
-- Ottaa käyttöön/poistaa "hiivailu"-tilan
body:set_crouching(enabled: bool)

-- Palauttaa fyysisen kehon tyypin (dynaaminen/kinemaattinen)
body:get_body_type() -> str
-- Asettaa fyysisen vartalon tyypin
body:set_body_type(type: str)
```

### Luuranko

Komponentti vastaa kokonaisuuden luurangosta. Katso [takila](../rigging.md).

```lua
-- Vähentäminen
local rig = entity.skeleton

-- Palauttaa luulle määritetyn indeksin mallin nimen
rig:get_model(index: int) -> str

-- Määrittää luumallin uudelleen määritetylle indeksille
-- Palauttaa alkuperäiseen, jos nimeä ei ole määritetty
rig:set_model(index: int, name: str)

-- Palauttaa luun muunnosmatriisin määritetyllä indeksillä
rig:get_matrix(index: int) -> mat4
-- Asettaa luun muunnosmatriisin määritettyyn indeksiin
rig:set_matrix(index: int, matrix: mat4)

-- Palauttaa pintakuvion avaimella (dynaamisesti määritetyt pintakuviot - '$name')
rig:get_texture(key: str) -> str

-- Määrittää tekstuurin näppäimellä
rig:set_texture(key: str, value: str)

-- Palauttaa luuindeksin nimellä tai nollan
rig:index(name: str) -> int

-- Tarkistaa luun näkyvyystilan indeksin mukaan
-- tai koko luuranko, jos indeksiä ei ole määritetty
rig:is_visible([optional] index: int) -> bool

-- Asettaa luun näkyvyystilan indeksin mukaan
-- tai koko luuranko, jos indeksiä ei ole määritetty
rig:set_visible([optional] index: int, status: bool)

-- Palauttaa kokonaisuuden värin
rig:get_color() -> vec3

-- Asettaa kokonaisuuden värin
rig:set_color(color: vec3)
```

## Komponenttitapahtumat

```lua
function on_despawn()
```

Kutsutaan, kun entiteetti poistetaan.

```lua
function on_grounded(force: number)
```

Soitettiin laskeutuessa. Ensimmäinen argumentti on iskuvoima. (Tällä hetkellä - nopeusmoduuli).

```lua
function on_fall()
```

Soittaa kun kaato alkaa.

```lua
function on_save()
```

Soitetaan ennen kuin komponenttitiedot tallennetaan. Täällä voit kirjoittaa tallennettavat tiedot *SAVED_DATA*-taulukkoon, joka on käytettävissä komponentin koko käyttöiän ajan.

```lua
function on_update(tps: int)
```

Kutsutaan jokaiselle entiteettien rastille (tällä hetkellä 20 kertaa sekunnissa).

```lua
function on_render(delta: number)
```

Kutsutaan joka kehys ennen entiteetin hahmontamista.

```lua
function on_sensor_enter(index: int, entity: int)
```

Kutsutaan, kun toinen entiteetti osuu anturiin, jonka indeksi on hyväksytty ensimmäisenä argumenttina. Anturin syöttäneen entiteetin UID välitetään toisena argumenttina.

```lua
function on_sensor_exit(index: int, entity: int)
```

Kutsutaan, kun toinen entiteetti poistuu anturista indeksi välitettynä ensimmäisenä argumenttina. Anturin poistuneen entiteetin UID välitetään toisena argumenttina.

```lua
function on_aim_on(playerid: int)
```

Kutsutaan, kun pelaaja tähtää kokonaisuuteen. Pelaajan tunnus välitetään argumenttina.

```lua
function on_aim_off(playerid: int)
```

Kutsutaan, kun pelaaja ottaa tavoitteen pois entiteetistä. Pelaajan tunnus välitetään argumenttina.

```lua
function on_attacked(attackerid: int, playerid: int)
```

Kutsutaan, kun entiteettiä vastaan ​​hyökätään (kokonaisuuden LMB). Ensimmäinen argumentti on hyökkäävän tahon UID. Hyökkäävän pelaajan tunnus välitetään toisena argumenttina. Jos pelaaja ei hyökännyt entiteettiin, toisen argumentin arvo on -1.


```lua
function on_used(playerid: int)
```

Kutsutaan, kun entiteettiä käytetään (RMB entiteetin mukaan). Pelaajan tunnus välitetään argumenttina.

