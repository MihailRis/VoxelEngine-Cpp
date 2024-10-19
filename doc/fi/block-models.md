# Block mallit

Voit luoda oman lohkomallin määrittämällä lohkolle seuraavat ominaisuudet:
```js
"model": "custom",
"model-primitives": {
  "aabbs": [
     // luettelo AABB-primitiivien kuvauksista
  ],
  // ... muut primitiivit
}
```

**AABB** primitiivinen - matriisi, joka koostuu arvoista:
```
[x, y, z, leveys, korkeus, syvyys, pintakuvioiden nimet kummallekin sivulle]
```

**tetragon** primitiivi (merkityksessään enemmän kuin suunnikas) - kolmen vektorin joukko, joka kuvaa primitiivin sijaintia, vektori X\*leveys, Y\*korkeus.
