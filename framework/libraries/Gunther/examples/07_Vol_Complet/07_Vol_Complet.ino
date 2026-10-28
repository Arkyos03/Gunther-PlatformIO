// Programme de vol complet :
//  - auto-test au démarrage (3 bips = OK)
//  - enregistrement de toutes les données sur la carte SD (vol_001.csv, vol_002.csv...)
//  - décollage détecté par le jack, apogée par le baromètre
//  - ouverture de la trappe à l'apogée, avec une sécurité à 8 s

#include <Gunther.h>

void setup() {
  Serial.begin(115200);
  Gunther.begin();
  Gunther.servoAngles(1, 0, 90);
  Gunther.servoClose(1);
  Gunther.setApogeeTimeout(8000);

  if (Gunther.selfTest() != GUNTHER_OK) {
    // une erreur : la LED 4 clignote, on ne va pas plus loin
    while (true) { Gunther.ledToggle(4); delay(200); }
  }

  Gunther.logBegin();               // nouveau fichier avec l'en-tête complet
  Gunther.logText("carte prete");
  Gunther.led(1, true);             // LED 1 = prête
}

void loop() {
  Gunther.update();
  Gunther.logFlight();              // une ligne : temps, état, altitude, accéléros, gyros...

  if (Gunther.apogee()) {
    Gunther.servoOpen(1);
    Gunther.led(3, true);
  }

  // une fois au sol (60 s après le décollage) on ferme le fichier
  if (Gunther.flightTime() > 60000) {
    Gunther.logText("fin du vol");
    Gunther.logClose();
    Gunther.beepCount(5);
    while (true) { delay(100); }
  }

  delay(10);                        // environ 100 lignes par seconde
}
