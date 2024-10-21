# Maailman Generaattori

## Sisältö

- [Peruskäsitteet](#peruskäsitteet)
- [Maailmanlaajuiset muuttujat](#globaalimuuttujat)
- [Asetustiedosto](#Asetustiedosto)
- [Fragments](#fragments)
- [Rakenteet](#rakenteet)
- [Biomes](#biomes)
 * [Biome-parametrit](#biomiparametrit)
 * [Biome-valinta](#biomivalinta)
- [Heightmap](#heightmap)
 * [Rakentaja](#constructor)
 * [Unary-operaatiot](#unary-operations)
 * [Binäärioperaatiot](#binääritoiminnot)
 * [heightmap:dump(...)](#heightmapdump)
 * [korkeuskartta:kohina(...)](#heightmapnoise)
 * [korkeuskartta:solukohina(...)](#heightmapcellnoise)
 * [korkeuskartta:resize(...)](#heightmapresize)
 * [korkeuskartta:crop(...)](#heightmapcrop)
 * [korkeuskartta:at(x, y)](#heightmap_at_x-y)
- [VoxelFragment (fragment)](#voxelfragment)
- [Korkeuskartan luominen](#korkeuskartan_luominen)
- [Rakenteiden manuaalinen järjestely](#manuaalinen-rakenteiden-järjestely)
 * [Rakenne-/tunnelisijoittelut](#rakenne-tunnelisijoittelut)
 * [Pienten rakenteiden järjestely](#Pienten-rakenteiden-järjestely)
 * [leveiden rakenteiden järjestely](#leveiden-rakenteiden-järjestely)
- [Rakenneilma](#rakenneilma)

## Peruskäsitteet

Alla tekstissä käytetyt käsitteet.

- **Yhdistettävä array/objekti** - TOML- tai JSON-tiedosto, joka on yhdistetty useista versioista eri paketeissa, jonka avulla voit lisätä siihen dataa ulkopuolelta. Yhdistetyn objektin kentät kirjoitetaan uudelleen järjestyksessä ensimmäisestä viimeiseen, kuten muutkin pakettien resurssit. Yhdistettävän taulukon tapauksessa kaksoiskappaleiden tarkistusta **ei** suoriteta.
- **Biome** - tieto, joka määrittää, mistä lohkoista ja missä kerroksissa maisema syntyy, sekä joukko kasveja ja rakenteita.
- **Plant** on pinnalle satunnaisesti sijoitettu lohko.
- **Pieni rakenne** - rakenne, jonka koko ei ylitä palan kokoa. Esimerkki: puut.

## Asetustiedosto

Maailmangeneraattori tunnistetaan tiedoston `generators/generator_name.toml` olemassaolosta. Muiden generaattoriin liittyvien tiedostojen tulee sijaita hakemistossa `generators/generator_name.files/`:
- biomes.toml - biomiilmoitukset
- structures.toml - rakenneilmoitukset
- script.lua - generaattorin komentosarja
- fragmentit - hakemisto, jossa fragmenttitiedostot sijaitsevat

Määritystiedostossa kuvatut perusominaisuudet:
- **caption** - generaattorin näyttönimi. Oletuksena se luodaan id:stä.
- **biome-parameters** - biomin valintaparametrien määrä (0 - 4). Oletus: 0.
- **sea-level** - merenpinta (tämän tason alapuolella ilman sijasta syntyy merikerroksia). Oletus: 0.
- **biomes-bpd** - lohkojen määrä biomin valintaparametrin karttapistettä kohti. Oletus: 4.
- **heights-bpd** - lohkojen lukumäärä korkeuskarttapistettä kohti. Oletus: 4.
- **wide-structs-chunks-radius** - suurin säde "leveiden" rakenteiden sijoittamiselle paloina mitattuna.

## Globaalimuuttujat

Seuraavat muuttujat ovat käytettävissä generaattoriskriptissä:

- `SEED` - maailman sukupolven vilja
- `__DIR__` - generaattorihakemisto (`pakkaus:generaattorit/generaattorin_nimi.files/`)
- `__FILE__` - komentosarjatiedosto (`pakkaus:generaattorit/generaattorin_nimi.files/script.lua`)

## Fragments

Fragmentti on myöhempää käyttöä varten tallennettu alue maailmassa, kuten pala, joka on rajoitettu tiettyyn leveyteen, korkeuteen ja pituuteen. Fragmentti voi sisältää tietoja alueelle joutuneiden lohkojen lisäksi myös alueen lohkojen inventaarioista sekä kokonaisuuksista. Toisin kuin pala, fragmentin koko on mielivaltainen.

Tässä vaiheessa fragmentti voidaan luoda komennolla `fragment.save` tai `generation.create_fragment`-funktiolla.

Generaattorin käyttämien fragmenttien on sijaittava hakemistossa:
`generators/generator_name.files/fragments/`

## Rakenteet

Rakenne - joukko sääntöjä fragmentin lisäämiseksi maailmaan generaattorilla. Tällä hetkellä sillä ei ole ominaisuuksia, vaan ne luodaan tyhjinä objekteina tiedostoon `generators/generator_name.files/structures.json`. Esimerkki:
```lua
{
    "tree0": {},
    "tree1": {},
    "tree2": {},
    "tower": {},
    "coal_ore0": {}
}
```

Tässä vaiheessa rakenteen nimen on vastattava käytetyn fragmentin nimeä.

## Biomes

Biomi määrittää, mistä lohkoista ja missä kerroksissa maisema syntyy, sekä kasveja ja rakenteita.

Biomit on ilmoitettu yhdistettävässä objektissa:
`generators/generator_name.files/biomes.toml`

Katsotaanpa biomin rakennetta metsän esimerkillä base:demo-generaattorista:

```toml
[forest]
parameters = [
    {weight=1, value=1},
    {weight=0.5, value=0.2}
]
layers = [
    {below-sea-level=false, height=1, block="base:grass_block"},
    {below-sea-level=false, height=7, block="base:dirt"},
    {height=-1, block="base:stone"},
    {height=1, block="base:bazalt"}
]
sea-layers = [
    {height=-1, block="base:water"}
]
plant-chance = 0.4
plants = [
    {weight=1, block="base:grass"},
    {weight=0.03, block="base:flower"}
]
structure-chance = 0.032
structures = [
    {name="tree0", weight=1},
    {name="tree1", weight=1},
    {name="tree2", weight=1},
    {name="tower", weight=0.002}
]
```

- avain forest - biomin nimi
- parameters - biomin parametrien painot ja keskiarvot. Katso kohta [biomin valinta] (#biomin valinta). Merkintöjen määrän on vastattava biomin valintaparametrien määrää.
- layers - lohkojen kerrokset ylhäältä alas.
 - height - kerroksen korkeus lohkoissa. -1:tä käytetään osoittamaan dimensioton (täyttö) kerros, joka voi olla vain yksi. Sen korkeus lasketaan automaattisesti.
 - block - koko lohkon nimi
 - below-sea-level alapuolella - voidaanko kerros luoda merenpinnan alapuolelle (esimerkki: turve). Jos arvo on epätosi, taso korvataan merenpinnan alapuolella luotaessa seuraavalla.
- sea-layers - valtameren kerrokset. Ylemmän kerroksen sijainti on sama kuin merenpinnan korkeus.
- plant-chance - todennäköisyys synnyttää kasvi pintalohkolle.
- plsnts - satunnaisesti pinnalle asetettuja kasveja.
 - weight - paino, joka vaikuttaa suoraan mahdollisuuteen valita tietty kasvi.
 - block - kasvilohko
- structure-chance - todennäköisyys luoda pieni rakenne pintalohkolle.
- structures - pinnalle satunnaisesti sijoitetut rakenteet.
 - name - "structures.json"-tiedoston ilmoitetun rakenteen nimi.
 - weight - paino, joka vaikuttaa suoraan mahdollisuuteen valita tietty rakenne.

### Biomiparametrit

Generaattoriparametri `biome-parameters` määrittää niiden valinnassa käytettyjen biomiparametrien määrän (esimerkkejä: lämpötila, kosteus).

Biomiparametriarvojen kartat luodaan samalla tavalla kuin korkeuskartat.

Toiminnon toteuttamiseen tarvitaan:
```lua
-- x, y - aloituspaikka kartalla (pisteinä)
-- w, k - kartan leveys ja korkeus (pikseleinä)
-- bpd - (lohkoja per piste) lohkojen määrä pistettä kohti (asteikko)
function generate_biome_parameters(x, y, w, h, bpd)
    -- korkeuskarttojen (Heightmap) luominen kullekin biomiparametrille
    --...
    return kortit_erotellaan_pilkuilla
end

-- Esimerkki:
function generate_biome_parameters(x, y, w, h, s)
    -- lämpötilakartta
    local tempmap = Heightmap(w, h)
    tempmap.noiseSeed = SEED + 5324
    tempmap:noise({x, y}, 0.04*s, 6)
    tempmap:pow(3)
    -- kosteuskartta
    local hummap = Heightmap(w, h)
    hummap.noiseSeed = SEED + 953
    hummap:noise({x, y}, 0.04*s, 6)
    hummap:pow(3)
    
    return tempmap, hummap
end
```

### Biomivalinta

Kun kullekin biomille on luotu parametrikartat, lasketaan arviot kaikille parametreille:

$pisteet = \frac{|V - V_b|}{W_b}$

Missä $V$ on parametrin arvo, $V_b$ on biomin keskusparametrin arvo, $W_b$ on parametrin biomin paino.

Generaattori valitsee biomin **pienimmän** parametripisteiden summan.


>[!WARNING]
> Jos parametrien arvot ja painot on määritetty väärin biomeille, voi esiintyä vaikutus, joka on luonteeltaan samanlainen kuin syvyysristiriita 3D-grafiikassa, kun kaksi pintaa on päällekkäin.
>
    Biomien tapauksessa kuvio näyttää satunnaiselta, koska parametrikarttojen luomiseen käytetty kohina vääristää näitä "pintoja".
    >
    Päästäksesi eroon vaikutuksesta, voit joko säätää biomiparametrien painoja tai arvoja tai lisätä eroa parametrikarttojen luomisessa.


## Heightmap

Heightmap on luokka korkeuskarttojen (mittalaisen kokoisten liukulukujen matriisien) kanssa työskentelemiseen.

### Constructor

Korkeuskartan rakentaja vaatii kokonaisluvun leveyden ja korkeuden.

```lua
local map = Heightmap(leveys, korkeus)
```

### Unary-operations

Toiminnot koskevat kaikkia korkeusarvoja.

```lua
map:abs()
```

Muuntaa korkeusarvot absoluuttisiksi arvoiksi.


### Binääritoiminnot

Toiminnot käyttämällä toista korttia tai skalaaria.

Aritmeettiset operaatiot:

```lua
-- Lisäys
map:add(value: Heightmap|number)

-- Vähennys
map:sub(value: Heightmap|number)

-- Kertominen
map:mul(value: Heightmap|number)

-- Eksponentointi
map:pow(value: Heightmap|number)
```

Muut toiminnot:

```lua
-- Minimi
map:min(value: Heightmap|number)

-- Maksimi
map:max(value: Heightmap|number)

-- Sekoitus
map:mixin(value: Heightmap|number, t: Heightmap|number)
-- t - sekoituskerroin 0,0 - 1,0
-- sekoitus suoritetaan seuraavan kaavan mukaan:
-- kartta_arvo * (1,0 - t) + arvo * t
```

### Heightmapdump

Virheenkorjaukseen käytetty menetelmä luo korkeuskarttaan perustuvan kuvan, joka muuntaa arvot alueelta `[-1.0, 1.0]` kirkkausarvoiksi `[0, 255]` ja tallentaa määritettyyn tiedostoon.

```lua
map:dump('export:test.png')
```

### Heightmapnoise

Menetelmä, joka tuottaa simpleksikohinaa lisäämällä sen olemassa oleviin arvoihin.

Kohinarae voidaan määrittää kentässä "map.noiseSeed".

```lua
map:noise(
    -- koordinaattipoikkeama
    offset: {number, number},
    -- koordinaattien skaalauskerroin
    scale: number,
    -- kohinan oktaavien määrä (oletus: 1)
    [valinnainen] octaves: integer,
    -- kohinan amplitudikerroin (oletus: 1,0)
    [valinnainen] multiplier: number,
    -- kartta X-koordinaattien siirtymistä melun synnyn aikana
    [valinnainen] shiftMapX: Heightmap,
    -- kartta Y-koordinaattien siirtymistä kohinan synnyn aikana
    [valinnainen] shiftMapY: Heightmap,
) -> nil
```

Melun visualisointi oktaaveilla 1, 2, 3, 4 ja 5.

![image](../images/simplex-noise.gif)

### Heightmapcellnoise

Highmap:noise-analogi, joka tuottaa matkapuhelinkohinaa.

Kohinarae voidaan määrittää kentässä `map.noiseSeed`.

![image](../images/cell-noise.gif)

### Heightmapresize

```lua
map:resize(leveys, korkeus, interpolointi)
```

Muuttaa korkeuskartan kokoa.

Käytettävissä olevat interpolointitilat:
- 'nearest' - ilman interpolointia
- 'linear' - bilineaarinen interpolointi

### Heightmapcrop

```lua
map:crop(x, y, leveys, korkeus)
```

Rajaa korkeuskartta määritetylle alueelle.

### Heightmap_at_x-y

```lua
map:at(x, y) --> number
```

Palauttaa korkeuden tietyssä paikassa.

## Voxelfragment

Fragmentti luodaan kutsumalla funktiota:
```lua
generation.create_fragment(
    -- piste A
    a: vec3,
    -- piste B
    b: vec3,
    -- Leikkaa fragmentti automaattisesti, jos mahdollista
    crop: bool
) -> VoxelFragment
```

Fragmentti voidaan ladata tiedostosta:
```lua
generation.load_fragment(
    -- Fragmentin tiedosto
    filename: str
) -> VoxelFragment
```

Fragmentti voidaan tallentaa tiedostoon:
```lua
generation.save_fragment(
    -- fragmentti tallennettava
    fragment: VoxelFragment,
    -- tiedosto
    filename: str
) -> nil
```

Fragmentin koko on saatavilla "koko"-ominaisuudena.

Fragmentti voidaan rajata niin, että se sopii sisältöön (ilma jätetään huomioimatta) kutsumalla `fragment:crop()` -menetelmää.

## Korkeuskartan_luominen

Oletusarvoisesti moottori luo korkeuskartan, joka koostuu nollista.

Mukautettujen korkeuskarttojen luomiseksi sinun on otettava käyttöön toiminto:
```lua
function generate_heightmap(
    x, y, -- korkeuskartan siirtymä
    w, h, -- Moottorin odottama korkeuskartan koko
    bpd,  -- lohkojen lukumäärä karttapistettä kohti (block per dot) - mittakaava
    [valinnainen] inputs - joukko biomiparametrien syöttökarttoja
    -- (katso generaattorin heightmap-inputs -ominaisuus)
) --> Heightmap
```

Esimerkki korkeuskartan luomisesta yksipuolisesta kohinasta valun avulla
halutulle alueelle:

```lua
function generate_heightmap(x, y, w, h, bpd)
    -- Luo tietyn kokoinen korkeuskartta
    local map = Heightmap(w, h)
    -- säädä melua
    map.noiseSeed = SEED
    -- kohina asteikolla 1/10 x 4 oktaavia ja amplitudi 0,5
    map:noise({x, y}, 0.1*bpd, 4, 0.5)
    -- siirrä korkeuksia positiiviselle alueelle
    map:add(0.5)
    return map
end
```

## Manuaalinen-rakenteiden-järjestely

### Rakenne-tunnelisijoittelut

Rakenne/rivisijoittelu on jouto annetuista
parametrien joukko.

Rakenne:
```lua
{rakenteen_nimi, rakenteen_sijainti, kierto, [valinnainen] prioriteetti}
```

Jossa:
- rakenteen_nimi - merkkijono, joka sisältää rakenne.toml-tiedostoon rekisteröidyn rakenteen nimen.
- rakenne_sijainti - vec3 (kolmen luvun joukko) suhteessa kappaleen sijaintiin.
- kierto - numero 0 - 3, joka osoittaa rakenteen pyörimisen Y-akselia pitkin.
- prioriteetti - numero, joka määrittää järjestyksen, jossa rakenteet asennetaan. Rakenteet, joilla on pienempi prioriteetti, menevät päällekkäin korkeamman prioriteetin omaavien rakenteiden kanssa.

Tunneli:
```lua
{":line", lohko_paikkamerkki, piste_a, piste_b, säde}
```

Jossa:
- lohko_paikkamerkki - sen lohkon numeerinen id, josta rakenne muodostuu.
- piste_a, piste_b - vec3, vec3 tunnelin alun ja lopun asemat.
- säde - tunnelin säde lohkoina


### Pienten-rakenteiden-järjestely

```lua
function place_structures(
    x, z, -- alueen alun sijainti lohkoissa
    w, d, -- alueen koko lohkoina
    heights, -- palan korkeuskartta
    chunk_height, -- palan korkeus
) --> rakenteen sijoitustaulukko
```

Rakenteet voidaan sijoittaa palan ulkopuolelle, mutta enintään yhden palan päähän.

Esimerkki:

```lua
function place_structures(x, z, w, d, hmap, chunk_height)
    local placements = {}
    local height = hmap:at(w/2, h/2) * chunk_height

    -- asettaa tornin palan keskelle
    table.insert(placements, {
        'tower', {w/2, height, d/2}, math.random() * 4, 2
    })
    return placements
end
```

### Leveiden-rakenteiden-järjestely

Rakenteet ja tunnelit voidaan sijoittaa kappaleen ulkopuolelle, mutta enintään leveät rakenteet-palaset-radius-generaattoriominaisuuden määrittämät kappaleet.

Toisin kuin edellinen toiminto, korkeuskartta ei lähetetä täällä,
koska puhelu tapahtuu kappaleiden luomisen alkuvaiheessa.

```lua
function place_structures_wide(
    x, z, -- alueen alun sijainti lohkoissa
    w, d, -- alueen koko lohkoina
    chunk_height, -- palan korkeus
) --> joukko rakenne-/tunnelisijoituksia
```

## Rakenneilma

`core:struct_air` - lohko, jota tulisi käyttää fragmentteina ilmaisemaan tyhjää tilaa, jota ei pitäisi täyttää lohkoilla, kun se syntyy maailmassa.

<image src="../../res/textures/blocks/struct_air.png" width="128px" height="128px" style="image-rendering: pixelated">
