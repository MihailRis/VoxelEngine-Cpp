# Suosituksia moottorin käyttöön

## Sisällön nimi

### Sisältöpakettejen ID

Sisältöpaketin tunnuksen on täytettävä seuraavat vaatimukset:
- nimi voi koostua vain latinalaisista kirjaimista, numeroista ja alaviivasta '\_'
- nimi ei voi alkaa numerolla
- Nimen pituuden on oltava välillä 2–24

### Lohkot ja Esineet

- Lohkojen ja kohteiden tunnukset noudattavat samoja vaatimuksia kuin sisältöpakettien tunnukset(id).
- `.item`-pääte lisätään vain korvaamaan lohkolle luotu alkio. Esimerkki: `base:stone.item` - kivikappaleelle luotu kohde.
- **caption**-kenttää, jonka tarkoituksena on näyttää nimike luettelossa, ei kirjoiteta isoilla kirjaimilla tarpeettomasti. Moottori lisää automaattisesti tapausta, kun se näkyy käyttöliittymässä.

## Tiedostojen tallennus

### Sisältöpaketin tiedot

#### Dataa maailmassa

Maailmassa pelastettavan tilan tulee olla muodossa `world:data/pack_id/`. Polku tulee saada erityisen toiminnon kautta:
```lua
local path = pack.data_file(PACK_ID, "tiedoston_nimi")
file.write(path, tiedot)
-- kirjoittaa tiedot tiedostoon world:data/PACK_ID/filename
```
Tässä PACK_ID on käytettävissä oleva vakio, eli sinun ei tarvitse kirjoittaa paketin nimeä itse.

Kansio `world:data/PACK_ID` luodaan, kun `pack.data_file` kutsutaan.

#### Yleistä tietoa

Asetusten ja muiden tietojen, joiden pitäisi olla saatavilla kaikista maailmoista, joissa pakkausta käytetään, on oltava
`config:pack_id/`. Voit käyttää erikoistoimintoa:

```lua
local path = pack.shared_file(PACK_ID, "tiedoston_nimi")
file.write(path, tiedot)
-- kirjoittaa tiedot tiedostoon config:PACK_ID/filename
```
