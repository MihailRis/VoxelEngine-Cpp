# Esilataa resurssit (tiedosto *preload.json*)

Automaattisesti latautumattomien resurssien, kuten äänien, lisätekstuurien, lataamiseen käytetään `preload.json` tiedostoa, joka luodaan sisältöpaketin kansioon.

Tiedoston omaisuus on jaettu luokkiin:
- fonts - fontit
- shaders - varjostimet
- textures - tekstuurit
- sounds - äänet
- models - mallit

> [!NOTE]
> Ääntä ladattaessa ladataan kaikki sen muunnelmat mallin mukaan:
> (ääni: äänen_nimi) -> *äänen_nimi.ogg, äänen_nimi_1.ogg, äänen_nimi_2.ogg, ...*
> tai *äänen_nimi_0.ogg, äänen_nimi_1.ogg, äänen_nimi_2.ogg, ...*

Äänen `pack:sounds/events/explosion.ogg` lisääminen kaikkine vaihtoehdoineen:
```json
{
    "sounds": [
        "events/explosion"
    ]
}
```
Tulee saataville nimellä "events/explosion"

В случае, если нужно будет работать с PCM данными звука (сейчас не доступно из скриптинга), требуется указать параметр `keep-pcm`:
```json
{
    "sounds": [
        {
            "name": "events/explosion",
            "keep-pcm": true
        }
    ]
}
```


Esimerkkitiedosto paketista `core:` (`res/preload.json`):
```json
{
    "shaders": [
        "ui3d",
        "screen",
        "background",
        "skybox_gen"
    ],
    "textures": [
        "misc/moon",
        "misc/sun",
        "gui/crosshair"
    ]
}
```
