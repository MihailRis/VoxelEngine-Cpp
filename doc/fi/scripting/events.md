# Moottoritapahtumat

## Lohko tapahtumat

Toiminnot lohkoskriptiin kirjoitettujen tapahtumien käsittelyyn.

```lua
function on_placed(x, y, z, playerid)
```

Kutsutaan sen jälkeen, kun pelaaja on asentanut lohkon

```lua
function on_broken(x, y, z, playerid)
```

Kutsutaan sen jälkeen, kun pelaaja tuhoaa lohkon

```lua
function on_interact(x, y, z, playerid) -> bool
```

Kutsutaan, kun napsautat RMB-lohkoa. Estää lohkojen asentamisen, jos palaa `true`

```lua
function on_update(x, y, z)
```

Kutsutaan, kun lohko päivitetään (jos viereinen lohko on muuttunut)

```lua
function on_random_update(x, y, z)
```

Laukaisee satunnaisina aikoina (ruoho kasvaa maapaloissa)

```lua
function on_blocks_tick(tps: int)
```

Soittaa tps (20) kertaa sekunnissa

## Kohde Tapahtumat

Toiminnot nimikekoodiin kirjoitettujen tapahtumien käsittelyyn.

```lua
function on_use(playerid: int)
```

Kutsutaan, kun napsautat hiiren kakkospainikkeella jotain muuta kuin lohkoa.

```lua
function on_use_on_block(x: int, y: int, z: int, playerid: int, normal: vec3)
```

Kutsutaan, kun napsautat hiiren kakkospainikkeella lohkoa. Estää kohdassa määritellyn lohkon asennuksen `placing-block` jos palauttaa `true`

```lua
function on_block_break_by(x: int, y: int, z: int, playerid: int)
```

Kutsutaan, kun napsautat LMB:tä lohkossa (mukaan lukien tuhoutumaton). Estää lohkon tuhoutumisen, jos palaa `true`

## Maailman tapahtumat  

Sisältöpaketin maailmantapahtumat on kirjoitettu sisään `scripts/world.lua`

```lua
function on_world_open()
```

Kutsutaan, kun maailma on ladattu

```lua
function on_world_save()
```  

Soitettiin ennen maailman pelastamista

```lua
function on_world_tick()
```

Soittaa 20 kertaa sekunnissa

```lua
function on_world_quit()
```

Soitettu, kun lähdet maailmasta (säästön jälkeen)
## Asettelutapahtumat

Tapahtumat kirjoitetaan tiedostoon `layouts/tiedoston_nimi.xml.lua`.

```lua
function on_open(invid: int, x: int, y: int, z: int)
```

Kutsutaan, kun elementti lisätään näytölle.
- Jos mainosjakaumaan ei ole linkkiä, invid on yhtä suuri kuin 0.
- Jos lohkoon ei ole linkkiä, x, y, z on myös yhtä kuin 0.

```lua
function on_close(invid: int)
```

Kutsutaan, kun elementti poistetaan näytöltä.

## HUD-tapahtumat  

Pelin käyttöliittymään liittyvät tapahtumat kirjoitetaan tiedostoon `scripts/hud.lua`

```lua
function on_hud_open(playerid: int)
```

Soitetaan maailmaan tullessaan, kun hud-kirjasto tulee saataville. Tässä näytölle lisätään pysyviä elementtejä.

```lua
function on_hud_close(playerid: int)
```

Soitetaan, kun poistutaan maailmasta, ennen kuin se tallennetaan.
