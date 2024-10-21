# Käsikirjoitus

Käytetään skriptikielinä LuaJIT

Alaosastot:
- [Moottorin tapahtumat](scripting/events.md)
- [Käyttäjän syöte](scripting/user-input.md)
- [Tiedostojärjestelmä ja serialisointi](scripting/filesystem.md)
- [Käyttöliittymäelementtien ominaisuudet ja menetelmät](scripting/ui.md)
- [Entiteetit ja komponentit](scripting/ecs.md)
- [Kirjastot](#)
 - [block](scripting/builtins/libblock.md)
 - [kamerat](scripting/builtins/libcameras.md)
 - [entiteetit](scripting/builtins/libentities.md)
 - [gui](scripting/builtins/libgui.md)
 - [hud](scripting/builtins/libhud.md)
 - [inventory](scripting/builtins/libinventory.md)
 - [tuote](scripting/builtins/libitem.md)
 - [mat4](scripting/builtins/libmat4.md)
 - [paketti](scripting/builtins/libpack.md)
 - [soitin](scripting/builtins/libplayer.md)
 - [quat](scripting/builtins/libquat.md)
 - [aika](scripting/builtins/libtime.md)
 - [vec2, vec3, vec4](scripting/builtins/libvecn.md)
 - [maailma](scripting/builtins/libworld.md)
- [Moduulin ydin:bit_converter](scripting/modules/core_bit_converter.md)
- [Moduulin ydin:data_puskuri](scripting/modules/core_data_buffer.md)
- [Moduulit ydin:vektori2, ydin:vektori3](scripting/modules/core_vector2_vector3.md)

## Tietotyyppimerkinnät

Lua-kirjastojen dokumentaatiossa käytetään tyyppimerkintöjä,
ei ole osa Lua-syntaksia.

- vector - kolmen tai neljän luvun joukko
- vec2 - kahden luvun joukko
- vec3 - kolmen luvun joukko
- vec4 - neljän luvun joukko
- quat - neljän luvun joukko - kvaternion
- matrix - 16 luvun joukko - matriisi

```lua
require "sisältöpaketti:moduulin_nimi" -- lataa lua-moduulin moduulikansiosta (laajennusta ei ole määritetty)
```
