# Sisältöpaketit

Sisältöpaketin luomiseksi sinun on ensin keksittävä nimi (id), joka täyttää seuraavat vaatimukset:
- nimi voi koostua vain latinalaisista kirjaimista, numeroista ja alaviivasta '\_'
- nimi ei voi alkaa numerolla
- Nimen pituuden on oltava välillä 2–24

Seuraavaksi luodaan *res/content*-kansioon kansio valitulla sisältöpaketin nimellä.

Luotuun kansioon luodaan tiedosto **package.json**, jonka sisältö on seuraava:
```json
{
    "id": "selected_pack_name",
    "title": "sisältövalikossa näytettävän sisältöpaketin nimi",
    "version": "sisältöpaketin versio major.minor-muodossa",
    "creator": "sisältöpaketin luoja",
    "description": "lyhyt kuvaus",
    "dependencies": [
        "riippuvuudet"
        "paketti"
    ]
}
```

Esimerkki:
```json
{
    "id": "doors",
    "title": "DOORS",
    "creator": "MihailRis",
    "version": "1.0",
    "description": "doors test"
}
```

Sisältöpaketin kuva lisätään *icon.png*-tiedostona paketin kansioon (ei pintakuvioihin). Suositeltu kuvakoko: 128x128

Uudet lohkot lisätään **blocks**-alikansioon, kohteet **items**, tekstuurit **textures**
Esimerkki tiedostorakenteesta on parempi tutustua peruspaketin kautta (*res/content/base*)
