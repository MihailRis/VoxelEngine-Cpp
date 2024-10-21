# Käyttöliittymäelementtien ominaisuudet ja menetelmät

Skriptien käyttöliittymäelementtejä käytetään asiakirja-ilmentymän kautta
(asiakirjamuuttuja) xml:ssä määritetyn id:n mukaan.

Esimerkki: elementin pos-ominaisuuden näyttäminen id: "worlds-panel" konsoliin:
```lua
print(document["worlds-panel"].pos)
-- tai
local worldsPanel = document["worlds-panel"]
print(worldsPanel.pos)
```

Elementtien tunnukset ovat dokumentille globaaleja, eli worlds-paneeli voi sijaita joko juurielementissä,
ja moninkertaisessa sisäkkäisessä säiliössä.

Elementin tunnusta ei voi muuttaa komentosarjasta.

Seuraavissa taulukoissa käytetään lyhennettyjä tyyppikuvauksia, kuten:
- vec2 - kahden murtoluvun joukko.
- ivec2 - kahden kokonaisluvun taulukko.
- rgba - neljän murtoluvun joukko alueella `[0..255]` tarkoittaa RGBA:n värejä.

Luan OOP-ominaisuuksien mukaan elementtimenetelmiä kutsutaan käyttämällä `:`-operaattoria `.`:n sijaan.

Esimerkiksi:
```lua
document["worlds-panel"]:clear()
```

## Yleiset ominaisuudet ja menetelmät

Ominaisuudet, jotka koskevat kaikkia elementtejä:

| Nimi      | Tyyppi    | Lukea | Kirjoittaa | Kuvaus                                  |
| ------------- | ------ | ------ | ------ | ----------------------------------------- |
| id            | string | joo     | *ei*  | elementin tunnus                    |
| pos           | vec2   | joo     | joo     | elementin sijainti säiliön sisällä        |
| wpos          | vec2   | joo     | joo     | elementin asema näytöllä                  |
| size          | vec2   | joo     | joo     | elementin koko                           |
| interactive   | bool   | joo     | joo     | kyky olla vuorovaikutuksessa elementin kanssa    |
| enabled       | bool   | joo     | joo     | visuaalisesti osoitettu interaktiivinen versio |
| visible       | bool   | joo     | joo     | elementin näkömyys                        |
| focused       | bool   | joo     | joo     | keskittyä elementtiin                         |
| color         | rgba   | joo     | joo     | elementin väri                             |
| hoverColor    | rgba   | joo     | joo     | leijuva väri                        |
| pressedColor  | rgba   | joo     | joo     | väri painettuna                          |
| tooltip       | string | joo     | joo     | työkaluvihjeteksti               |
| tooltipDelay  | float  | joo     | joo     | työkaluvihjeen viive            |
| contentOffset | vec2   | joo     | *ei*  | sisällön offset                      |

Yleiset elementtimenetelmät:

| Menetelmä               | Kuvaus                                                                |
| ------------------- | ----------------------------------------------------------------------- |
| moveInto(container) | siirtää elementin määritettyyn säilöön (elementti on määritetty, ei id) |
| destruct()          | poistaa elementin                                                         |

## Kontit

Yleiset menetelmät säilöille (elementit: säiliö, paneeli, painike, sivulaatikko):

| Menetelmä                           | Kuvaus                                                                                    |
| ------------------------------- | ------------------------------------------------------------------------------------------- |
| clear()                         | siivoo контент                                                                             |
| add(xml)                        | lisää elementin luomalla sen xml-koodilla. Esimerkki: `container:add("<image src='test'/>")` |
| setInterval(interval, callback) | нmäärittää toiminnon, joka suoritetaan toistuvasti millisekunteina määritetyllä aikavälillä         |

## Tekstikenttä (textbox)

Ominaisuudet:

| Nimi   | Tyyppi    | Lukea | Kirjoittaa | Kuvaus                                                               |
| ----------- | ------ | ------ | ------ | ---------------------------------------------------------------------- |
| text        | string | joo     | joo     | syötetty teksti tai paikkamerkki                                        |
| placeholder | string | joo     | joo     | paikkamerkki (käytetään, jos mitään ei ole syötetty)                 |
| caret       | int    | joo     | joo     | vaunun asento. `textbox.caret = -1` asettaa paikan tekstin loppuun |
| editable    | bool   | joo     | joo     | tekstin muuttuvuus                                                    |
| multiline   | bool   | joo     | joo     | monilinjainen tuki                                              |
| textWrap    | bool   | joo     | joo     | automaattinen tekstin rivitys (vain monirivinen: "true")           |
| valid       | bool   | joo     | ei    | Onko syötetty teksti oikein?                                 |

Menetelmät:

| Menetelmä       | Kuvaus                                     |
| ----------- | -------------------------------------------- |
| paste(text) | lisää määritetyn tekstin merkintäkohtaan |

##  Liukusäädin (trackbar)

Ominaisuudet:

| Nimi   | Tyyppi   | Lukea | Kirjoittaa | Kuvaus                     |
| ---------- | ----- | ------ | ------ | ---------------------------- |
| value      | float | joo     | joo     | valittu arvo           |
| min        | float | joo     | joo     | minimiarvo         |
| max        | float | joo     | joo     | enimmäisarvo        |
| step       | float | joo     | joo     | jakovaihe                  |
| trackWidth | float | joo     | joo     | ohjauselementin leveys |
| trackColor | rgba  | joo     | joo     | ohjausväri   |

## Valikko (pagebox)

Ominaisuudet:

| Nimi | Tyyppi    | Lukea | Kirjoittaa | Kuvaus         |
| -------- | ------ | ------ | ------ | ---------------- |
| page     | string | joo     | joo     | tämä sivu |

Menetelmät:

| Menetelmä   | Kuvaus                                   |
| ------- | ------------------------------------------ |
| back()  | siirtyy edelliselle sivulle            |
| reset() | nollaa sivun ja vaihtohistorian |

## Valintaruutu (checkbox)

Ominaisuudet:

| Nimi | Tyyppi  | Lukea | Kirjoittaa | Kuvaus          |
| -------- | ---- | ------ | ------ | ----------------- |
| checked  | bool | joo     | joo     | merkitse tila |

## Painike (button)

Ominaisuudet:

| Nimi | Tyyppi    | Lukea | Kirjoittaa | Kuvaus     |
| -------- | ------ | ------ | ------ | ------------ |
| text     | string | joo     | joo     | Painikkeen teksti |

## Label (label)

Ominaisuudet:

| Nimi | Tyyppi    | Lukea | Kirjoittaa | Kuvaus    |
| -------- | ------ | ------ | ------ | ----------- |
| text     | string | joo     | joo     | Label teksti |

## Kuva (image)

Ominaisuudet:

| Nimi | Tyyppi    | Lukea | Kirjoittaa | Kuvaus              |
| -------- | ------ | ------ | ------ | --------------------- |
| src      | string | joo     | joo     | renderöity tekstuuri |

## Varasto (inventory)

Ominaisuudet:

| Nimi  | Tyyppi | Lukea | Kirjoittaa | Kuvaus                                  |
| --------- | --- | ------ | ------ | ----------------------------------------- |
| inventory | int | joo     | joo     | sen varaston tunnus, johon tuote on linkitetty |

