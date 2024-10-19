# Kohteiden ominaisuudet

## Näkökulma

### Kuvakkeen tyyppi - `icon-type` ja itse kuvake - `icon`

Moottorin uusimmassa versiossa on seuraavan tyyppisiä tuotekuvakkeita, jotka määrittävät, kuinka tuote näytetään luettelossa:
- `none` - näkymätön tyyppi, käytetään vain "core:empty" (tyhjä kohde). Ei vaikuta kohteen ulkoasuun sisällön käyttöpaneelissa. Tyyppi voidaan poistaa tulevaisuudessa
- `sprite` - 2D-kuva. Vaaditaan kuvakeominaisuus, joka koostuu atlasen nimestä ja siinä olevan tekstuurin nimestä erotettuna merkillä `:`. Esimerkki: `blocks:notfound`. Tällä hetkellä moottorissa on kaksi tekstuuriatrasta:
 - blocks (luodettu png-tiedostoista hakemistossa `res/textures/blocks/`)
 - items (luodettu png-tiedostoista hakemistossa `res/textures/items/`)
- `Block` - näyttää lohkon esikatselun. Kuvake määrittää näytettävän lohkon merkkijonotunnuksen. Esimerkki `base:wood`

## Käyttäytyminen

### Asennettava lohko - `placing-block`

Jos määrität merkkijonolohkon tunnuksen(id), kohde asettaa sen, kun painat RMB-näppäintä. Tätä ominaisuutta käytetään kaikissa lohkoille luoduissa kohteissa.

Esimerkki: esine asettaa basalttilohkoja:

```json
"placing-block": "base:bazalt"
```

### Säteily - `emission`

Vaikuttaa esineen lähettämään valoon, kun se on pelaajan kädessä.

Kolmen kokonaisluvun joukko - R, G, B, valaistus 0-15.

Esimerkkejä:

- `[15, 15, 15]` - kirkkain valkoinen valo
- `[7, 0, 0]` - heikko punainen valo
- `[0, 0, 0]` - kohde ei lähetä valoa (oletus)

### Pinon koko - `stack-size`

Määrittää tuotteen enimmäismäärän yhdessä paikassa. Oletusarvo on 64.
