# Konsoli

Komentotulkin kanssa työskentelemistä varten on tarjolla kirjasto **console**.

## Komennon luominen

Luo konsolikomento käyttämällä seuraavaa toimintoa:

```python
console.add_command(järjestelmä: str, toimeenpanija: function)
```

Kaavalla on seuraava syntaksi:

```
nimeä sijaintiparametrit {nimetyt parametrit}
```

Otsikko voi sisältää:
- Latinalaiset aakkoset
- numerot (paitsi ensimmäinen merkki)
- `.`, `_`, `-`

Paikkaparametrit erotetaan välilyönnillä ja niillä on seuraava syntaksi:

```
nimi:tyyppi (vaihtoehto 1)
nimi:tyyppi=oletus (vaihtoehto 2)
nimi:tyyppi~keskusarvo (vaihtoehto 3)
nimi:tyyppi=oletus~keskiarvo (vaihtoehto 4)
```

Saatavilla olevat tyypit:
- **int** - kokonaisluku
- **num** - murtoluku
- **str** - merkkijono
- **sel** - valitsin (objektin tunnus esitetään kokonaisluvulla)
- **enum** - luettelo

Vaihtoehdot 3 ja 4 näyttävät `~`-operaattorin, jonka avulla voit käyttää suhteellisia arvoja. *Keskiarvo* - arvo, johon käyttäjä määritetään. Esimerkiksi pelaajan asema.

Suhteellinen operaattori toimii vain numeroiden kanssa (num tai int)

**console.set**:n kautta määritetyt muuttujat voidaan määrittää keskeisiksi arvoiksi.

Esimerkki:

```python
x:num~pos.x
```

Muuttujat voidaan myös määrittää oletusarvoiksi etuliitettä käytettäessä `$`:

```python
t:int=$time
```

Siirrot on ilmoitettu muodossa:

```python
mode:[replace|destruct|none]
```

Tai muuttujan kautta:

```python
mode:enum $modes
```

Valitsimet määritetään `@`-etuliitteellä. Tällä hetkellä ne ovat tynkä esinemallin puutteen vuoksi. Pitäisi tehdä valinnainen ja käyttää muuttujia:

```python
obj:sel=$obj.id # obj.id - pelaajan id
```

Nimetyt argumentit määritetään erityisessä lohkossa, joka on erotettu kaarevilla aaltosulkeilla `{ }` saman mallin mukaisesti.

Esimerkki:

```python
eval name:str="World" {greeting:str='Hello'}
```

## Esimerkkejä komentokaaviosta

Vakiokomentomallit löytyvät tiedostosta `res/script/stdcmd.lua`.

Esimerkki - komento **tp**:

```python
tp obj:sel=$obj.id x:num~pos.x y:num~pos.y z:num~pos.z
```

Täysi lua-koodi komennon luomiseen:

```lua
console.add_command(
    "tp obj:sel=$obj.id x:num~pos.x y:num~pos.y z:num~pos.z",
    "Teleport object",
    function (args, kwargs)
        player.set_pos(unpack(args))
    end
)
```

- Paikkaargumenttien valmiit arvot välitetään args:ille.
- Nimettyjen argumenttiarvojen taulukko välitetään kargs:ille.

Komentojen tulkki suorittaa tyypin tarkistuksen ja suoratoiston automaattisesti.
