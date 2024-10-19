# Kokonaisuuden ominaisuudet

## Logiikka

### Luettelo komponenteista - *components*

Määrittää komponentit ja järjestyksen, jossa ne alustetaan.

```json
"components": [
    Luettelo komponenteista
]
```

Esimerkki:

```json
"components": [
    "base:drop"
]
```

Komponenttikoodin tulee olla kohdassa `scripts/components`.

## Fysiikka

### Hitbox - *hitbox*

Kolmen numeron joukko, joka ilmaisee entiteetin osumalaatikon koon.

Esimerkki:

```json
"hitbox": [0.6, 1.8, 0.6]
```


### Vartalotyyppi - *body-type*

Määrittää, kuinka fysiikan moottori toimii sen kanssa.

- *static* (staattinen) - fysiikan moottori ei vaikuta kehoon.
- *dynamic* (dynaaminen) - oletustyyppi. Fysiikkamoottori laskee liikkeet ja törmäykset.
- *kinematic* (kinemaattinen) - vain liike lasketaan ilman törmäyksiä.

### Estäminen - *blocking*

Määrittää, estääkö entiteetti lohkojen asennuksen.

*Tulevaisuudessa myös estää muiden yksiköiden liikkumisen.*

Oletusarvo: *true*.

### Luettelo antureista - *sensors*

Anturi on fyysiseen kehoon kiinnitetty avaruusalue, joka havaitsee muiden kappaleiden pääsyn siihen.

- Kun kappale osuu, *on_sensor_enter* -tapahtuma laukeaa.
- Poistuessaan kehosta kutsutaan *on_sensor_exit* -tapahtuma.

Anturit indeksoidaan samassa järjestyksessä kuin ne esitetään luettelossa, alkaen 0:sta.

Seuraavat anturityypit (muodot) on olemassa:
- radius (säde) on yksinkertaisin anturi. Määrittää osumalaatikon keskustan ympärillä olevan alueen. Seuraavat arvot on määritelty:
 - säde - numero.
- aabb (laatikko) - suorakaiteen muotoinen alue, joka muuttaa sijaintia kokonaisuuden pyörimisestä riippuen. **Pala itsessään ei pyöri.** Seuraavat arvot on määritetty:
 - kolme numeroa x, y, z alueen vähimmäiskulmasta.
 - kolme numeroa x, y, z alueen vastakkaisesta kulmasta.

Esimerkki:

```json
"sensors": [
    ["aabb", -0.2, -0.2, -0.2, 0.2, 0.2, 0.2],
    ["radius", 1.6]
]
```

0. Suorakaiteen muotoinen alue, jonka leveys, korkeus ja pituus on 0,4 m ja jonka keskipiste on 0,0.
1. Säteittäinen alue, jonka säde on 1,6 m.

## Näkökulma

### Skeleton nimi - *skeleton-name*

Oletusarvo on sama kuin entiteetin nimi. Määrittää, mitä luurankoa entiteetti käyttää. Katso [takila](rigging.md).

## Tallennetaan/Ladataan

Näiden mukautettujen komponenttien lisäksi moottori tallentaa automaattisesti sisäänrakennettujen komponenttien tiedot: muunnos, jäykkä runko, luuranko.

On joukko lippuja, joiden avulla voit määrittää, mitkä tiedot tallennetaan ja mitkä eivät.

(Totuusarvot on määritetty)

| Nimi               | Kohde                                                     | Tavallisesti |
| ---------------------- | -------------------------------------------------------- | ------------ |
| save                   | itse olemus                                            | true         |
| save-skeleton-pose     | luurankoasento olemus                                    | false        |
| save-skeleton-textures | dynaamisesti osoitettu texturit                         | false        |
| save-body-velocity     | matkanopeus vartalo                                   | true         |
| save-body-settings     | muutettu kehon asetuksia <br>(type, damping, crouching) | false        |
