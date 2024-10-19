# Resurssit

Resursseja ovat:
- kamerat
- paikat tehosteille
- kehyspuskurit
- ja vastaavat määrältään rajoitetut resurssit

Tällä hetkellä vain toteutettu **kamerat**.

Pakkauksen pyytämät resurssit määritetään resources.json-tiedoston kautta muodossa:
```json
{
    "resurssityyppi": [
        "resurssien",
        "nimet"
    ]
}
```

Paketin lataamisen jälkeen resurssien nimet saavat paketin etuliiteen. Esimerkiksi kamera
*Cinematic* peruspaketissa on nimeltään *base:cinematic*.
