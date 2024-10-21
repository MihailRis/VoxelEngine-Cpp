# Kirjasto *gui*

Kirjasto sisältää toimintoja käyttöliittymäelementtien ominaisuuksiin pääsemiseksi. Guin sijasta sinun tulee käyttää objektikäärettä, joka tarjoaa pääsyn ominaisuuksiin __index, __newindex metamenetelmien kautta:

```lua
print(document.some_button.text) -- missä 'some_button' - elementin id
document.some_button.text = "uusi teksi"
```

Asetteluskriptissä `layouts/layout_file.xml` - `layouts/layout_file.xml.lua` muuttuja **document**, joka sisältää Document-luokan objektin, on jo saatavilla.

```python
gui.str(text: str, context: str) -> str
```

Palauttaa käännetyn tekstin.

```python
gui.get_viewport() -> {int, int}
```

Palauttaa pääsäiliön (ikkunan) koon.

```python
gui.get_env(document: str) -> table
```

Palauttaa määritetyn asiakirjan ympäristön (maailmanlaajuisten muuttujien taulukon).

```python
gui.get_locales_info() -> taulukko taulukoista missä
 avain - aluetunnus isolangcode_ISOCOUNTRYCODE-muodossa
 arvo - taulukko {
       name: str # alueen nimi sen kielellä
   }
```

Palauttaa tiedot kaikista ladatuista alueista (res/texts/\*).
