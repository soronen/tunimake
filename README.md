# Tehtävänanto:



Ohjelma käynnistetään komennolla tunimake [kuvaustiedosto]. Jos kuvaustiedostoa ei anneta, niin sen oletetaan olevan nimellä tunimakefile työskentelyhakemistossa.

Kuvaustiedostossa on omilla riveillään kuvauksia lähdekooditiedostoista, joiden avulla tuotetaan ajettava ohjelma (kuten makefilessä). Tekstirivit voivat olla:

    Tyhjä rivi (sisältää vain ns. whitespace-merkkejä)
    Rivi, joka alkaa #-merkillä, joka on kommentti
    K kääntäjä Käytettävän kääntäjän nimi (esim. gcc)
    F liput Kääntäjälle annettavat lisäliput
    C kooditiedosto-lista Lista on pilkulla eroteltu lista tiedostonimiä, nämä ovat käännettävät lähdekooditiedostot.
    H header-lista Käytetyt otsikko/header -tiedostot
    L kirjasto-lista Mukaan otettavat kirjastot (-lkirjastonimi)
    E ajettavan exe-tiedoston nimi 

Perustoiminnassa C-tiedostoista käännetään yksitellen objekteja (.o), joista lopuksi kirjastojen kanssa käännetään ajettava binääri.

Mitään ei käännetä turhaan uudestaan. Jos objekti on jo olemassa ja sitä vastaava c-tiedosto ei ole muuttunut (tiedoston päiväykset), niin käännöstä ei tarvita. Erikoistapauksena jos mikä tahansa otsikkotiedosto on muuttunut, niin kaikki käännetään uudelleen.

Jos tarvittavia tietoja ei ole olemassa (kääntäjän nimiä, nolla lähdekoodia, yms.), niin ohjelma lopettaa virheilmoituksella toimintansa.

Vinkkejä: stat() kutsulla saa tietoa tiedostosta. system() -kutsulle ei kannata suoraan antaa syötettä käyttäjältä (tunimakefilestä). Turvallisempi vaihtoehto on fork() + exec().

Esimerkki:
```text
K g++
F -pthread
C saikeet.cc
H saikeet.hh
L pthread
E saietesti
#
# ajoon lähtee komento:
# g++ -pthread saikeita.cc -pthread -o saietesti
```text
