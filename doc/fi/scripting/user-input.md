# Käyttäjän syöttö

Näppäinpainallusten ja hiiren painikkeiden käsittely käsitellään sidoksilla, jotka on määritetty paketissa `config/bindings.toml`-tiedostossa muodossa:

```toml
packid.binding.name="inputtype:codename"
```

- pakattu - valinnainen, mutta suositeltavaa
- syöttötyyppi - näppäin tai hiiri
- koodinimi - näppäimen tai hiiren painikkeen nimi (left/right/middle)

## Avainten_nimet

- space, backspace, tab, enter, caps-lock, escape
- left-ctrl, left-shift, left-alt, left-super
- right-ctrl, right-shift, right-alt, right-super
- delete, home, end, insert, page-up, page-down
- left, right, down, up
- a..z
- 0..9
- f1..f25

## Kirjasto input

```python
input.keycode(keyname: str) -> int
```

Palauttaa avainkoodin nimellä tai -1

```python
input.mousecode(mousename: str) -> int
```

Palauttaa hiiren painikkeen koodin nimellä tai -1

```python
input.add_callback(bindname: str, callback: function)
```

Määrittää toiminnon, jota kutsutaan, kun sidonta aktivoidaan. Esimerkki:
```lua
input.add_callback("hud.inventory", function ()
	print("Inventory open key pressed")
end)
```

```python
input.get_mouse_pos() -> {int, int}
```

Palauttaa kursorin sijainnin näytöllä.

```python
input.get_bindings() -> joukko merkkijonoja
```

Palauttaa kaikkien käytettävissä olevien sidosten nimet.

```python
input.is_active(bindname: str) -> bool
```

Tarkistaa, onko sidonta aktiivinen.

```python
input.is_pressed(code: str) -> bool
```

Tarkistaa syöttötoiminnan käyttämällä koodia, joka koostuu seuraavista:
- syöttötyyppi: näppäin (näppäin) tai hiiri (hiiren painike)
- syöttökoodi: [näppäimen nimi](#Avainten_nimet) tai hiiren painikkeen nimi (vasen, keskimmäinen, oikea)

Esimerkki: 
```lua
if input.is_pressed("key:enter") then
    ...
end
```
