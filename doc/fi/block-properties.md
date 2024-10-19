# Block ominaisuudet

## Näkökulma

### Tekstuuri - *texture*

Lohkon tekstuurin nimi (vain nimi ilmoitetaan, ilman päätettä tai tiedostopolkua)

Tekstuuritiedoston on oltava sisällä `res/textures/blocks/` ja niillä on muoto **png**

### Sivujen teksturointi - *texture-faces*

>[!IMPORTANT]
> Ei voida käyttää samanaikaisesti *texture* kanssa

6 tekstuurin nimen joukko, jonka avulla voit määrittää ne kummallekin puolelle erikseen.

Esimerkki: 
```json
"texture-faces": [
    "grass_side",
    "grass_side",
    "dirt",
    "grass_top",
    "grass_side",
    "grass_side"
]
```

### Malli - *model*

Estä malli luettelosta:
- "block" - oletuksena käytössä kaikille tavallisille lohkoille
- "none" - näkymätön lohko (esimerkki: ilma)
- "X" - ruohomalli (kahden spritin risti)
- "aabb" - lohkohittilaatikkoa vastaava malli (yhdistetty hitbox yhdistetään yhdeksi). Esimerkkejä: putket, hehkulamput, paneelit.

### Renderöinti ryhmä - *draw-group*

Kokonaisluku, joka määrittää tämän lohkon renderöintiryhmän numeron. 
Koskee läpikuultavia lohkoja - ratkaisee tämän lohkon takana olevien lohkojen näkymättömien sivujen ongelman.

### Kierto - *rotation*

Kiertoprofiili (joukko paikkoja, joihin lohko voidaan asentaa) luettelosta:

- "none" - lohkon kierto on poissa käytöstä (oletuksena)
- "pipe" - "putki" -profiili. Esimerkkejä lohkoista: tukki, putki, hehkulamppu
- "pane" - "paneeli" -profiili. Esimerkkejä lohkoista: paneeli, ovi, kyltti

## Valaistus

### Säteily - *emission*:

Kolmen kokonaisluvun joukko - R, G, B, valaistus 0-15.

Esimerkit:

- `[15, 15, 15]` - kirkkain valkoinen valo
- `[7, 0, 0]` - heikko punainen valo
- `[0, 0, 0]` - lohko ei lähetä valoa (oletus)


### Valonjohtavuus - *light-passing*

Kun asetus on `true`, lohko johtaa valoa lähettävistä lohkoista.

### Auringon läpäisevyys - *sky-light-passing*

Kun asetus on `true` lohko ei estä pystysuoran auringonvalonsäteen kulkua.


### Ei valaistusta - *shadeless*

Sammuttaa lohkomallin valaistuksen.

### Huippu Ambient-Occlusion - *ambient-occlusion*

Määrittää, onko kärkipisteen AO vaikutus olemassa. Oletuksena käytössä.

## Fysiikka

### Este - *obstacle*

False poistaa lohkon hitboxin käytöstä (sallii pelaajan kävellä lohkon läpi)

### Osumisboksi - *hitbox*

6 numeron joukko, joka kuvaa lohkon hitboxin siirtymää ja kokoa.

Numerot on ilmoitettu alueella `[0,0, 1,0]` - eli lohkon sisällä.

Taulukko `[0,25, 0,0, 0,5, 0,75, 0,4, 0,3]` kuvaa osumalaatikkoa:
- siirtynyt 0,25 m länteen
- siirretty 0,0 m ylöspäin
- siirtynyt 0,5 m pohjoiseen
- leveys (idästä länteen) 0,75 m
- Korkeus 0,4 m
- pituus (etelästä pohjoiseen) 0,3 m

### Maahan kosketus - *grounded*

Lohko voidaan asettaa vain täysiksi lohkoksi.
Tuhoutuu, kun sen alla oleva lohko tuhoutuu.

### Erottuva - *selectable*

Kun asetus on `false` kohdistin ohittaa lohkon ja korostaa sen takana olevan lohkon.

### Vaihdettavuus - *replaceable*

Kun asetus on `true` Lohkon tilalle voit asentaa minkä tahansa muun lohkon. Esimerkki: vesi, ruoho, kukka.

### Tuhoavuus - *breakable*

Kun asetus on `false` lohkoa ei voi rikkoa.

## Varasto

### Piilotettu lohko - *hidden*

Kun asetus on `true` lohko ei näy varastossa eikä sille luoda tuotetta, joten arvosta 0.17 lähtien sinun on määritettävä *picking-item*-ominaisuus

### Nouto kohde - *picking-item*

Kohde, joka valitaan, kun napsautat lohkoa keskipainikkeella.

Esimerkki: `door:door_open` -lohko on piilotettu, joten `picking-item: "door:door.item"` on määritetty

### Skriptin nimi - *script-name*

Voit määrittää lohkokomentosarjan nimen. Ominaisuus tarjoaa mahdollisuuden käyttää yhtä komentosarjaa useille lohkoille.
Nimi määritetään ilman `pack:scripts/` ja laajennusta.

### Käyttöliittymän asettelun nimi - *ui-layout*

Voit määrittää lohkorajapinnan XML-asettelun tunnuksen. Oletusarvoisesti käytetään merkkijonolohkotunnusta.

### Varaston koko - *inventory-size*

Lohkovarastopaikkojen määrä. Oletus - 0 (ei varastoa)

## Laajennetut lohkot

### Lohkon koko - *size*

Kolmen kokonaisluvun taulukko. Oletusarvo - `[1, 1, 1]`.

## Estä kentät

Lohkokenttien avulla voit tallentaa enemmän lohkokohtaisia ​​tietoja kuin käyttäjäbitit sallivat.

Estä kentät on ilmoitettu seuraavassa muodossa:

```json
"fields": {
    "nimi": {"type": "tietotyyppi"},
    "listan_nimi": {"type": "tietotyyppi", "length": "taulukonpituus"}
}
```

Parametrien `type` ja `length` lisäksi käytettävissä on `convert-strategy` -parametri, joka määrittää
arvon muunnosstrategiaa tietotyyppiä rajattaessa.

Parametri saa yhden kahdesta arvosta:
- `reset` - arvo, joka ei ole uudella alueella, nollataan
- `clamp` - arvo pienennetään lähimpään uudella alueella

Esimerkki: numero 231, kun kenttätyyppiä vaihdetaan int16:sta int8:ksi:
- `reset`-tilassa se muuttuu 0:ksi
- `Clamp`-tilassa se muuttuu 127:ksi


Käytettävissä olevat tietotyypit:

| Tyyppi     | Koko   | Kuvaus                      |
| ------- | -------- | ----------------------------- |
| int8    | 1 tavu   | etumerkillinen kokonaisluku 8 bitti  |
| int16   | 2 tavua  | etumerkillinen kokonaisluku 16 bitti |
| int32   | 4 tavua  | etumerkillinen kokonaisluku 32 bitti |
| int64   | 8 tavu   | etumerkillinen kokonaisluku 64 bitti |
| float32 | 4 tavua  | todellinen 32 bitti           |
| float64 | 8 tavua   | todellinen 64 bitti           |
| char    | 1 tavu   | symbolinen                    |

- Tällä hetkellä kenttäkokojen yhteissumma ei saa ylittää 240 tavua.
- Kenttä, jossa ei ole määritetty taulukon pituutta, vastaa 1 elementin taulukkoa.
- Merkkityyppistä taulukkoa voidaan käyttää UTF-8-merkkijonojen tallentamiseen.
