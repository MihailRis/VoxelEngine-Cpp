# XML-rajapinnan merkintä

Katso myös [ui-elementit komentosarjassa](scripting/ui.md).

# Tietyt tyypit

**2D-vektori** on pilkulla erotettu numeropari.
Esimerkkejä:
- `"500,200"`
- `"0.4,53.01"`
- `"0,0"`

**3D-vektori** – kolme pilkulla erotettua numeroa.
Esimerkkejä:
- `"60,30,53"`
- `"0.4,0.1,0.753"`

**4D-vektori** – neljä pilkulla erotettua numeroa.
Esimerkkejä:
- `"10,5,10,3"`
- `"0.1,0.5,0.0,0.0"`

**RGBA-väri** - tällä hetkellä vain HEX-tallennus on käytettävissä.
Esimerkkejä:
- `"#FF8000"` - oranssi läpinäkymätön
- `"#FFFFFF80"` - valkoinen läpikuultava
- `"#000000FF"` - musta läpinäkymätön

# Yleiset elementin attribuutit

- `enablen` - kun se on epätosi, estää elementin, toisin kuin interaktiivinen, ja osoittaa tämän tilan visuaalisesti.
- `id` - elementin tunniste. Tyyppi: merkkijono.
- `pos` - elementin sijainti. Tyyppi: 2D-vektori.
- `size` - elementin koko. Tyyppi: 2D-vektori.
- `context` - määrittää käännöskontekstin "@"-merkkijonoille.
- `color` - elementin väri. Tyyppi: RGBA-väri.
- `hover-color` - elementin väri, kun osoitinta pidetään. Tyyppi: RGBA-väri.
- `pressed-color` - elementin väri, kun elementtiä painetaan. Tyyppi: RGBA-väri.
- `margin` - elementin ulkoinen marginaali. Tyyppi: 4D-vektori.
 Järjestys: "vasen, ylhäällä, oikea, alhaalla".
- `visible` - elementin näkyvyys. Tyyppi: Boolen ("tosi"/"false").
- `position-func` - elementin sijainnin tarjoaja (kaksi numeroa), jota kutsutaan, kun elementin sijaintisäiliön kokoa muutetaan tai kun elementti lisätään säilöön. Voidaan kutsua ennen kuin on_hud_open kutsutaan.
- `size-func` - elementin koon tarjoaja (kaksi numeroa), jota kutsutaan, kun elementin sijaintisäiliön koko muuttuu tai kun elementti lisätään säilöön. Voidaan kutsua ennen kuin on_hud_open kutsutaan.
- `onclick` - lua-funktio kutsutaan kun elementtiä napsautetaan.
- `ondoubleclick` - lua-funktio kutsutaan, kun kaksoisnapsautat elementtiä.
- `tooltip` - työkaluvihje teksti
- `tooltip-delay` - ​​viive työkaluvihjeen ilmestymisessä
- `gravity` - elementin automaattinen sijoittaminen säiliöön. (Ei toimi automaattisissa säiliöissä, kuten paneeli). Arvot: *ylävasen, ylhäällä-keskellä, ylhäällä oikea, keskellä vasen, keski-keski, keskioikea, alhaalla-vasen, alhaalla-keskellä, alhaalla-oikealla*.
- `z-index` - määrittää elementtien järjestyksen. Suuremmalla arvolla se menee päällekkäin pienempien elementtien kanssa.
- `interactive` - jos arvo on epätosi, hiiren osoitin elementin ja kaikkien alielementtien päällä ohitetaan.

# Mallin attribuutit

- `if` arvoilla ('', 'false', 'nil') elementti ohitetaan, mukaan lukien alielementit.
- `ifnot` on sama kuin `if`, mutta päinvastaisella ehdolla.

# Yleiset säilön attribuutit

Säiliöissä on myös paneeleja ja painikkeita.
- `padding` - elementin sisäinen täyttö. Tyyppi: 4D-vektori.
 Järjestys: "vasen, ylhäällä, oikea, alhaalla".
- `scrollable` - vieritysmahdollisuus. Toimii vain paneelissa. Tyyppi: Boolen.

# Yleiset paneeliattribuutit

Paneeleissa on myös painikkeet.
- `max-length` - enimmäispituus, jolla paneelia venytetään ennen vierityksen alkamista (jos vieritettävä = tosi). Tyyppi: numero
- `orientation` - paneelin suunta: vaaka/pysty.

# Peruselementit

## Painike - *button*

Sisäteksti - painikkeen teksti.

- `text-align` - tekstin tasaus ("left", "center" tai "right"). Tyyppi: merkkijono.

## Valintaruutu - *checkbox*

- `checked` - määrittää merkin tilan.
- `supplier` - merkitse tilan tarjoaja (kutsutaan jokaiseen kehykseen)
- `consumer` - lua lippuvaltion vastaanotintoiminto.

## Merkki - *label*

- `valign` - tekstin pystytasaus: top/center/bottom
- `supplier` - tekstintarjoaja (kutsutaan jokaiseen kehykseen)
- `autoresize` - automaattinen elementin koon muuttaminen (oletus - false). Ei vaikuta kirjasinkokoon.
- `multiline` - Mahdollistaa monirivisen tekstin näyttämisen.
- `text-wrap` - sallii automaattisen tekstin rivityksen (toimii vain monirivillä: "true")

## Kuva - *image*

- `src` - pintakuvioiden kansiossa olevan kuvan nimi määrittämättä laajennusta. Tyyppi: merkkijono. Esimerkiksi `gui/error`

## Teksti kenttä - *textbox*

Sisäteksti - alun perin syötetty teksti

- `placeholder` - jokerimerkkiteksti (käytetään, jos tekstikenttä on tyhjä)
- `supplier` - tekstintarjoaja (kutsutaan jokaiseen kehykseen)
- `consumer` - lua-toiminto, joka vastaanottaa syötetyn tekstin. Soitetaan vain, kun syöttö on valmis.
- `autoresize` - automaattinen elementin koon muuttaminen (oletus - false). Ei vaikuta kirjasinkokoon.
- `multiline` - Mahdollistaa monirivisen tekstin näyttämisen.
- `text-wrap` - sallii automaattisen tekstin rivityksen (toimii vain monirivillä: "true")
- `editable`- määrittää, voidaanko tekstiä muokata.
- `error-color` - väriä syötettäessä vääriä tietoja (teksti ei läpäise validaattorin tarkistusta). Tyyppi: RGBA-väri.
- `validator` - lua-toiminto, joka tarkistaa tekstin oikeellisuuden. Ottaa syötteenä merkkijonon, palauttaa tosi, jos teksti on oikein.
- `onup` - lua-toiminto kutsutaan, kun ylänuolta painetaan.
- `ondown` - lua-toiminto kutsutaan, kun alanuolta painetaan.

## Liukusäädin - *trackbar*

- `min` - minimiarvo. Tyyppi: numero. Oletus: 0
- `max` - enimmäisarvo. Tyyppi: numero. Oletus: 1
- `value` - alkuperäinen merkitys. Tyyppi: numero. Oletus: 0
- `step` - Liukusäätimen jaon koko. Tyyppi: numero. Oletus: 1
- `track-width` - osoittimen leveys (pikseleinä). Tyyppi: numero. Oletus: 12
- `track-color` - Osoittimen leijuva väri. Tyyppi: RGBA-väri.
- `consumer` - lua asetettu arvon vastaanotintoiminto
- `sub-consumer` - lua väliarvon vastaanotintoiminto (käytä tekstin päivittämiseen, kun `change-on-release="true"`)
- `supplier` - lua-arvon tarjoajatoiminto
- `change-on-release` - Kuluttajatoimintoa kutsutaan vain, kun käyttäjä vapauttaa osoittimen. Tyyppi: Boolen. Oletus: false

# Varastotuotteet

## Varasto - *inventory*

Elementti on säiliö. Tällä hetkellä ei ole erityisiä määritteitä. 

> [!WARNING]
> Varastojen sijaintia ohjaa moottori, eikä sitä voi muuttaa sijainnin, marginaalin jne. ominaisuuksilla.

## Yksi paikka - *slot*

Elementin on oltava sisällä `inventory` elementti, ilman välittäjiä.
- `index` - varastopaikan indeksi. (Numerointi 0:sta)
- `item-source` - mahdollistaa sisältöpaneelin kaltaisen toiminnan. Tyyppi: Boolen
- `sharefunc` - lua-tapahtuma laukeaa käytettäessä LMB + Vaihto. Varaston tunnus ja paikkaindeksi lähetetään.
- `updatefunc` - lua-tapahtuma kutsutaan, kun paikan sisältö muuttuu.
- `onrightclick` - lua-tapahtuma laukeaa käytettäessä RMB:tä. Varaston tunnus ja paikkaindeksi lähetetään.

## Slot ruudukko - *slots-grid*

Elementin on oltava sisällä `inventory` elementti, ilman välittäjiä.
- `start-index` - ensimmäisen paikan indeksi.
- `rows` - rivien määrä (ei määritetty, jos sarakkeet on määritetty).
- `cols` - sarakkeiden määrä (ei määritetty, jos rivit on määritetty).
- `count` - paikkojen kokonaismäärä (ei määritetty, jos rivit ja sarakkeet on määritetty).
- `interval` - aikaväli. Tyyppi: numero.
- `padding` - tilaa aukkojen ruudukon ympärillä. Tyyppi: numero. (*määrite poistetaan*)
- `sharefunc` - lua-tapahtuma laukeaa käytettäessä LMB + Vaihto. Varaston tunnus ja paikkaindeksi lähetetään.
- `updatefunc` - lua-tapahtuma kutsutaan, kun paikan sisältö muuttuu.
- `onrightclick` - lua-tapahtuma laukeaa käytettäessä RMB:tä. Varaston tunnus ja paikkaindeksi lähetetään.
