# Rigging

## Luurankoja

Entiteettirungot luodaan json-tiedostoilla skeletons-kansiossa.

> [!IMPORTANT]
> 
> Luuranko on ei-indeksoitava sisältöyksikkö. Ladattaessa pakkauksen etuliite lisätään nimeen (esimerkki: *drop* peruspaketissa -> *base:drop*).

Luurankoelementti eli luu koostuu muunnosmatriisista, joka määrittää sen sijainnin, kierron ja mittakaavan suhteessa emoelementtiin (luun) tai kokonaisuuteen, jos elementti on juuri, mallista ja alielementtien luettelosta.

Runkotiedostolla on seuraava rakenne:
```json
{
    "root": {
        "name": "nimi",
        "model": "mallin_nimi",
        "offset": [x, y, z],
        "nodes": [
           ...
        ]
    }
}
```

- root - juurielementti
- name - elementin nimi indeksin saamiseksi (kenttä valinnainen)
- model - mallin nimi, jolla elementti näytetään (kenttä valinnainen)
- offset - elementin siirtymä suhteessa pääelementtiin (kenttä valinnainen)
- nodes - luettelo elementeistä - jälkeläiset, joihin tämän elementin matriisi vaikuttaa (kenttä on valinnainen)

Tällä hetkellä paikannus, rotaatio, skaalaus tehdään skriptauksella sekä animaatiolla.

Luurankojen kanssa työskentelyprosessia yksinkertaistetaan tulevaisuudessa.

Mallit ladataan automaattisesti; niitä ei tarvitse lisätä preload.json-tiedostoon.

## Mallit

Mallien tulee sijaita mallit-kansiossa. Tällä hetkellä vain OBJ-muotoa tuetaan.

>[!IMPORTANT]
> Obj-mallia ladattaessa \*.mtl-tiedosto ohitetaan.

 Pintakuvio määritellään materiaalinimellä, joka vastaa preload.json-tiedostossa käytettyä tekstuurin nimeämismuotoa.

 Tekstuurit ladataan automaattisesti, mallin käyttämiä pintakuvioita ei tarvitse määrittää preload.jsonissa.