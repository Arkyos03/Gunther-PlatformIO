// Ejection du parachute par servo à l'apogée
//  - le jack de décollage est sur le bornier JACK
//  - la carte détecte le décollage (jack arraché) puis l'apogée (l'altitude redescend)
//  - sécurité : si l'apogée n'est pas vue, la trappe s'ouvre quand même après 8 s

#include <Gunther.h>

void setup() {
  Gunther.begin();
  Gunther.servoAngles(1, 0, 90);    // servo 1 : fermé à 0°, ouvert à 90°
  Gunther.servoClose(1);
  Gunther.setApogeeTimeout(8000);   // ouverture forcée 8 s après le décollage
  Gunther.selfTest(false);          // 3 bips = carte prête (sans carte SD)
}

void loop() {
  Gunther.update();

  if (Gunther.liftoff()) Gunther.led(1, true);

  if (Gunther.apogee()) {
    Gunther.servoOpen(1);
    Gunther.led(2, true);
    Gunther.beep(3000, 1000);
    while (true) { delay(100); }    // fin du vol (delay garde l'USB actif)
  }
}
