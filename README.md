<div align="center">

# Gunther.ino pour PlatformIO

**Les cartes électroniques des fusées d'AIR ESIEA, programmables avec PlatformIO (VS Code)**

*by Arkyos03 for AIR ESIEA*

![PlatformIO](https://img.shields.io/badge/PlatformIO-6.x-F5822A?logo=platformio&logoColor=white)
![Licence](https://img.shields.io/badge/licence-MIT-green)

</div>

---

## C'est quoi ?

Ce dépôt permet de programmer les cartes Gunther.ino avec PlatformIO. C'est exactement le même code
que dans l'Arduino IDE (même cœur, mêmes noms de broches, même bibliothèque `Gunther`) : un programme
écrit dans l'un marche dans l'autre.

Tu préfères l'Arduino IDE ? Tout est dans le dépôt [Gunther-Arduino](https://github.com/Arkyos03/Gunther-Arduino).

---

## Les cartes

| Carte | Pour quelle fusée | Nom PlatformIO | Où en est-on |
|---|---|---|---|
| **Gunther.ino (Rev A)** | Minifusée | `gunther` | Disponible |
| *Carte expérimentale* | Fusée expérimentale | – | En préparation |

---

## Utiliser la carte dans un projet

Dans le `platformio.ini` de ton projet :

```ini
[env:gunther]
platform = https://github.com/Arkyos03/Gunther-PlatformIO.git#v0.1.5
board = gunther
framework = arduino
```

PlatformIO télécharge tout seul la plateforme et le compilateur au premier build.
Pour passer à une nouvelle version, change juste le numéro après `#`.

**Téléverser** : par USB, rien à ajouter. Avec une sonde SWD, ajoute une de ces lignes :

```ini
upload_protocol = cmsis-dap      ; Raspberry Pi Debug Probe, Atmel-ICE, DAPLink...
upload_protocol = stlink         ; ST-Link V2
```

**Déboguer** (points d'arrêt, registres du SAMD51) : avec une sonde SWD, ajoute
`debug_tool = cmsis-dap` ou `debug_tool = stlink`, puis lance le débogage dans VS Code.

---

## Programmer la carte

Comme dans l'Arduino IDE, deux façons de faire :

**Le mode simplifié** : `#include <Gunther.h>` et chaque action tient en une ligne.

```cpp
#include <Arduino.h>
#include <Gunther.h>

void setup() {
  Gunther.begin();          // démarre la carte et prend l'altitude du sol
  Gunther.servoClose(1);    // trappe fermée
  Gunther.selfTest();       // 3 bips = tout va bien
  Gunther.logBegin();       // nouveau fichier vol_XXX.csv sur la carte SD
}

void loop() {
  Gunther.update();
  Gunther.logFlight();      // une ligne de données par tour
  if (Gunther.apogee()) Gunther.servoOpen(1);   // on ouvre le parachute
}
```

**Le mode classique** : code Arduino standard (`pinMode`, `digitalWrite`, `Wire`, `SPI`...) avec les noms
des broches de la carte (`LED_1`, `JACK_1`, `SERVO_1`, `BUZZER`...).

Sous PlatformIO, pense à mettre `#include <Arduino.h>` en haut de ton `main.cpp`.

Les exemples sont dans `framework/libraries/Gunther/examples` (mode simplifié) et
`framework/libraries/Gunther_Classique/examples` (mode classique) : copie le `.ino` dans `src/main.cpp`.

---

## Documentation

La fiche de la carte (brochage, fonctions, couleurs de la LED) et les manuels d'utilisation sont dans
le dépôt [Gunther-Arduino](https://github.com/Arkyos03/Gunther-Arduino).

---

## En cas de problème

**Le téléversement ne marche plus** (programme planté, port introuvable) : appuie deux fois rapidement
sur **RESET**. La carte passe en mode bootloader : relance le téléversement.

Pour toute autre question, contacte Arkyos03 ou l'association : air-esiea@et.esiea.fr

---

<div align="center">

Licence MIT · Arkyos03 for AIR ESIEA

</div>
