# Tiedostojärjestelmä ja serialisointi

## Kirjasto *file*

Toimintojen kirjasto tiedostojen käsittelyä varten

```python
file.resolve(polku: str) -> str
```

Toiminnon tulosten syöttö `точка_входа:путь` (esimerkiksi `user:worlds/house1`) tavalliselle tielle. (esimerkiksi `C://Users/user/.voxeng/worlds/house1`)

> [!NOTE]
> Toimintoa ei tarvitse käyttää yhdessä muiden kirjaston toimintojen kanssa, koska ne tekevät tämän automaattisesti

Palautettu polku ei ole kanoninen ja voi olla joko absoluuttinen tai suhteellinen.

```python
file.read(polku: str) -> str
```

Lukee koko tekstitiedoston ja palauttaa sen merkkijonona

```python
file.read_bytes(polku: str) -> array of integers
```

Lukee tiedoston tavutaulukkoon.

```python
file.write(polku: str, текст: str) -> nil
```

Kirjoittaa tekstiä tiedostoon (korvaamalla)

```python
file.write_bytes(polku: str, data: array of integers)
```

Kirjoittaa tiedostoon tavutaulukon (korvaamalla)

```python
file.length(polku: str) -> int
```

Palauttaa tiedoston koon tavuina tai -1, jos tiedostoa ei löydy

```python
file.exists(polku: str) -> bool
```

Tarkistaa, onko tietyllä polulla tiedosto tai hakemisto

```python
file.isfile(polku: str) -> bool
```

Tarkistaa, onko tietyllä polulla tiedosto olemassa

```python
file.isdir(polku: str) -> bool
```

Tarkistaa, onko tietyllä polulla hakemisto

```python
file.mkdir(polku: str) -> bool
```

Luo hakemiston. Palauttaa tosi, jos uusi hakemisto on luotu

```python
file.mkdirs(polku: str) -> bool
```

Luo koko hakemistoketjun. Palauttaa tosi, jos hakemistoja on luotu.

```python
file.find(polku: str) -> str
```

Etsii tiedostoa viimeisestä paketista res. Polku on määritetty ilman etuliitettä. Palauttaa polun halutulla etuliitteellä. Jos tiedostoa ei löydy, palauttaa arvon nolla.

```python
file.remove(polku: str) -> bool
```

Poistaa tiedoston. Palauttaa **true**, jos tiedosto oli olemassa. Tekee poikkeuksen, kun käyttöoikeusrikkomus tapahtuu.

```python
file.remove_tree(polku: str) -> int
```

Rekursiivisesti poistaa tiedostoja. Palauttaa poistettujen tiedostojen määrän.

## Kirjasto json

Kirjasto sisältää toimintoja taulukoiden sarjoittamista ja sarjojen poistamista varten:

```python
json.tostring(object: table, human_readable: bool=false) -> str
```

Sarjaa objektin JSON-merkkijonoon. Jos toisen parametrin arvoksi on asetettu **true**, käytetään monirivistä muotoilua, joka on kätevä ihmisen luettavaksi, oletuksena olevan kompaktin muotoilun sijaan.

```python
json.parse(code: str) -> table
```

Jäsentää JSON-merkkijonon taulukkoon.

## Kirjasto toml

Kirjasto sisältää toimintoja taulukoiden sarjoittamista ja sarjojen poistamista varten:

```python
toml.tostring(object: table) -> str
```

Sarjaa objektin TOML-merkkijonoon.

```python
toml.parse(code: str) -> table
```

Jäsentää TOML-merkkijonon taulukoksi.

## Kirjasto bjson

Kirjasto sisältää toimintoja binääritiedonvaihtoformaatin kanssa työskentelemiseen [vcbjson](../../specs/binary_json_spec.md).

```lua
-- Koodaa taulukon tavutaulukkoon
bjson.tobytes(
    -- koodattu taulukko
    value: table, 
    -- pakkaus
    [valinnainen] compression: bool=true
) --> Bytearray

-- Purkaa tavutaulukon taulukoksi
bjson.frombytes(bytes: table | Bytearray) --> table
```

## Datan tallennus maailmassa

Kun tallennat pakettidataa maailmaan, sinun tulee käyttää toimintoa
```python
pack.data_file(pack_id: str, tiedoston_nimi: str) -> str
```

Funktio palauttaa polun datatiedostoon tyypin mukaan: `world:data/pack_id/tiedoston_nimi`
ja luo matkan varrella puuttuvia hakemistoja.

Käytettäessä polkuja, jotka eivät ole sopivia `data/{packid}/...` Mahdollinen tietojen menetys, kun maailmaa korvataan.
