# Kirjasto *pack*

```python
pack.is_installed(packid: str) -> bool
```

Tarkistaa asennetun paketin olemassaolon maailmassa

```lua
pack.data_file(packid: str, filename: str) -> str
-- ja
pack.shared_file(packid: str, filename: str) -> str
```

Palauttaa polun datatiedostoon
ja luo matkan varrella puuttuvia hakemistoja.

- Ensimmäinen vaihtoehto palauttaa: `world:data/packid/filename`
- Toinen vaihtoehto palauttaa: `config:packid/filename`

Esimerkkejä:
```lua
file.write(pack.data_file(PACK_ID, "example.txt"), text)
```
Pakkauksessa *containermod* kirjoittaa tekstin tiedostoon `world:data/containermod/example.txt`.

Käytä tietojen tallentamiseen maailmassa.

```lua
file.write(pack.shared_file(PACK_ID, "example.txt"), text)
```
Pakkauksessa *containermod* kirjoittaa tekstin tiedostoon `config:containermod/example.txt`

Käytä kaikkien maailmojen yhteisten tietojen tallentamiseen.

```python
pack.get_folder(packid: str) -> str
```

Palauttaa polun asennetun sisältöpaketin kansioon.

```python
pack.is_installed(packid: str) -> bool
```

Tarkistaa sisältöpaketin olemassaolon maailmassa

```python
pack.get_installed() -> joukko merkkijonoja
```

Palauttaa kaikkien maailmassa asennettujen sisältöpakettien tunnukset.

```python
pack.get_available() -> joukko merkkijonoja
```

Palauttaa kaikkien saatavilla olevien mutta ei asennettujen sisältöpakettien tunnukset.

```python
pack.get_base_packs() -> joukko merkkijonoja
```

Palauttaa kaikkien peruspakettien tunnukset (ei poistettavissa)

```python
pack.get_info(packid: str) -> {
	id: str,
	title: str,
	creator: str,
	description: str,
	version: str,
	icon: str,
	dependencies: valinnainen joukko merkkijonoja
}
```

Palauttaa tiedot paketista (ei välttämättä asennettu).
- kuvake - esikatselutekstuurin nimi (latautuu automaattisesti)
- riippuvuudet - merkkijonot muodossa `{lvl}{id}`, jossa lvl:
  - `!` - pakollinen
  - `?` - valinnainen
  - `~` - heikko
  esimerkiksi `!teal`.
