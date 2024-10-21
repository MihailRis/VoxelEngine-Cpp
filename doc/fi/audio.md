# Audio

## Peruskäsitteet

### Backend (Backend)

Muunnos audioalijärjestelmän sisäisestä toteutuksesta, joka ohjaa äänilähtöä.
Tällä hetkellä moottorissa on kaksi:
- NoAudio - tynkä, jota käytetään, kun OpenAL:n alustaminen on mahdotonta tai kun äänijärjestelmä on poistettu käytöstä asetustiedoston kautta: `[audio] enabled=false`. Tämä taustaohjelma lataa PCM-dataa vain pyynnöstä eikä luo kaiuttimia yrittäessään toistaa ääntä.
- ALAaudio on päävaihtoehto. Äänilähtö OpenAL:n kautta.

### Kanava (Channel)

Määrittää äänilähteiden luokan äänenvoimakkuuden säätöä, tehosteita ja taukoa varten.
Tällä hetkellä on olemassa seuraavat kanavat:
- master - ohjaa muiden kanavien äänenvoimakkuutta. Ei pitäisi määrittää kohdekanavaksi ääntä toistettaessa.
- ui - käyttöliittymän äänet
- regular - pelimaailman äänet keskeytetään pelin mukana.
- ambient - sama kuin tavallinen, mutta tarkoitettu taustaäänille: sää ja muu ympäristö.
- music - kanava musiikin toistamiseen. Yleensä suoratoistaa ääntä.

Kanavia ohjaa itse moottori.
### Kaiutin (Speaker)

Toistettavan äänen kerta-ohjain: ääni tai suoratoisto. Kaiutin tuhoutuu pysähtymisen jälkeen kutsumalla **stop**-menetelmään tai kun ääni loppuu (lankaa ei myöskään suojata tuhoamasta kaiutinta).
Säädin jatkaa toimintaansa keskeytettynä.

> [!NOTE]
Kaiuttimiin päästään kokonaislukutunnuksilla, jotka eivät toistu moottorin toiminnan aikana. Suorien osoittimien tallentamista luokkaobjekteihin tulee välttää.

Kaiutintunnusten numerointi alkaa 1:stä. ID 0 tarkoittaa, että toisto on jostain syystä mahdotonta.
### Ääni (Sound)

Muistiin ladattu äänidata mahdollistaa samanaikaisen toiston useista lähteistä. Voi tarjota pääsyn PCM-tietoihin.

### PCM lähde (PCMStream)

Virta, jota virta käyttää PCM-tietojen lähteenä. Toteutus ei riipu äänijärjestelmän taustajärjestelmästä, vaan tiedostomuodosta. Äänen suoratoisto verkkoyhteydestä tapahtuu tämän rajapinnan toteutuksen kautta.

### Virtaus (Stream)

Äänen suoratoisto. Sitä ei ladata kokonaan muistiin, joten se ei vaadi esilatausta `preload.json`-tiedoston kautta. Ei voi toistaa useista kaiuttimista samanaikaisesti.

## Muototuki

Tällä hetkellä kahden muodon tuki on otettu käyttöön.
- WAV: 8 ja 16 bitin tuki (OpenAL ei tue 24 bittiä)
- OGG: toteutettu libvorbis-kirjaston kautta


## Lisäksi

> [!WARNING]
> Kun toistetaan OpenAL:n kautta, stereoäänet eivät ota huomioon lähteiden sijaintia suhteessa soittimeen. Äänien, joiden on otettava sijainti huomioon, tulee olla mono.

## Audio API komentosarjoissa

### Toistaa ääntä

Äänenkäsittely tehdään kirjaston `audio` kanssa.

```lua
audio.play_stream(
    -- polku äänitiedostoon
    name: string, 
    -- äänilähteen sijainti maailmassa
    x: number, y: number, z: number,
    -- äänenvoimakkuus (0,0 - 1,0)
    volume: number
    -- toistonopeus (positiivinen numero)
    pitch: number,
    -- [valinnainen] kanavan nimi: tavallinen/ambient/musiikki/ui (oletus - tavallinen)
    channel: string,
    -- [valinnainen] streamin silmukka (oletus - epätosi)
    loop: bool
) -> int
```

Toistaa suoratoistoääntä määritetystä tiedostosta määritetyssä paikassa maailmassa. Palauttaa kaiuttimen tunnuksen(id).

```lua
audio.play_stream_2d(
    -- polku äänitiedostoon
    name: string, 
    -- äänenvoimakkuus (0,0 - 1,0)
    volume: number
    -- toistonopeus (positiivinen numero)
    pitch: number,
    -- [valinnainen] kanavan nimi: tavallinen/ambient/musiikki/ui (oletus - tavallinen)
    channel: string,
    -- [valinnainen] streamin silmukka (oletus - epätosi)
    loop: bool
) -> int
```

Toistaa äänen suoratoistona määritetystä tiedostosta. Palauttaa kaiuttimen tunnuksen(id).


```lua
audio.play_sound(
    -- ladatun äänen nimi ilman paketin etuliitettä, "äänet/", versionumeroa ja päätettä 
    -- esimerkki "steps/stone" toistaa ääntä, joka on ladattu osoitteesta "sounds/steps/stone.ogg" tai mistä tahansa sen muunnelmista
    -- äänivaihtoehto valitaan satunnaisesti
    name: string, 
    -- äänilähteen sijainti maailmassa
    x: number, y: number, z: number,
    -- äänenvoimakkuus (0,0 - 1,0)
    volume: number
    -- toistonopeus (positiivinen numero)
    pitch: number,
    -- [valinnainen] kanavan nimi: tavallinen/ambient/musiikki/ui (oletus - tavallinen)
    channel: string,
    -- [valinnainen] streamin silmukka (oletus - epätosi)
    loop: bool
) -> int
```

Toistaa äänen määritetyssä paikassa maailmassa. Palauttaa kaiuttimen tunnuksen(id).

```lua
audio.play_sound_2d(
    -- ladatun äänen nimi ilman paketin etuliitettä, "äänet/", versionumeroa ja päätettä 
    -- esimerkki "steps/stone" toistaa ääntä, joka on ladattu osoitteesta "sounds/steps/stone.ogg" tai mistä tahansa sen muunnelmista
    -- äänivaihtoehto valitaan satunnaisesti
    name: string, 
    -- äänenvoimakkuus (0,0 - 1,0)
    volume: number
    -- toistonopeus (positiivinen numero)
    pitch: number,
    -- [valinnainen] kanavan nimi: tavallinen/ambient/musiikki/ui (oletus - tavallinen)
    channel: string,
    -- [valinnainen] streamin silmukka (oletus - epätosi)
    loop: bool
) -> int
```

Toistaa ääntä. Palauttaa kaiuttimen tunnuksen(id).

### Vuorovaikutus puhujan kanssa.

Kun käytät olemattomia kaiuttimia, mitään ei tapahdu.

```lua
-- lopeta kaiuttimen toisto
audio.stop(speakerid: integer)

-- keskeyttää kaiuttimen
audio.pause(speakerid: integer)

-- palauta kaiuttimen tauko
audio.resume(speakerid: integer)

-- aseta äänisilmukka
audio.set_loop(speakerid: integer, state: bool)

-- tarkista, onko ääni silmukka (epätosi, jos sitä ei ole)
audio.is_loop(speakerid: integer) -> bool

-- hanki kaiuttimen äänenvoimakkuus (0,0, jos sitä ei ole)
audio.get_volume(speakerid: integer) -> number

-- Aseta kaiuttimen äänenvoimakkuus
audio.set_volume(speakerid: integer, volume: number)

-- hanki toistonopeus (1.0, jos sitä ei ole)
audio.get_pitch(speakerid: integer) -> number

-- Aseta toistonopeus
audio.set_pitch(speakerid: integer, pitch: number)

-- saada äänen aikapaikka sekunneissa (0,0, jos sitä ei ole)
audio.get_time(speakerid: integer) -> number

-- Aseta äänen ajan sijainti sekunneissa
audio.set_time(speakerid: integer, time: number)

-- saada äänilähteen sijainti maailmassa (nolla, jos sitä ei ole)
audio.get_position(speakerid: integer) -> number, number, number

-- aseta äänilähteen sijainti maailmassa
audio.set_position(speakerid: integer, x: number, y: number, z: number)

-- saada äänilähteen nopeus maailmassa (nolla, jos sitä ei ole)
-- (OpenAL käyttää Doppler-ilmiön simulointiin)
audio.get_velocity(speakerid: integer) -> number, number, number

-- aseta äänilähteen nopeus maailmassa
-- (OpenAL käyttää Doppler-ilmiön simulointiin)
audio.set_velocity(speakerid: integer, x: number, y: number, z: number)

-- saada lähteen toistaman äänen kesto sekunteina
-- palauttaa 0:n, jos kaiutinta ei ole
-- palauttaa myös 0, jos kestoa ei tiedetä (esimerkki: radio)
audio.get_duration(speakerid: integer) -> number
```

### Muut ominaisuudet

```lua
-- Hanki nykyinen live-kaiuttimien määrä
audio.count_speakers() -> integer

-- saada nykyinen toistettavien äänivirtojen määrä
audio.count_streams() -> integer
```
