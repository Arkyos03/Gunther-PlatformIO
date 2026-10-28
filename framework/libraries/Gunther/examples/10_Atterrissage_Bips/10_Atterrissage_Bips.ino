// Vol avec détection de l'atterrissage et lecture de l'altitude au buzzer :
//  - enregistrement sur la carte SD jusqu'à l'atterrissage
//  - une fois au sol, la carte bipe l'altitude max en boucle
//    (437 m = 4 bips, pause, 3 bips, pause, 7 bips ; un bip long = 0)

#include <Gunther.h>

void setup() {
  Serial.begin(115200);
  Gunther.begin();
  Gunther.servoAngles(1, 0, 90);
  Gunther.servoClose(1);
  Gunther.setApogeeTimeout(8000);   // sécurité : ouverture forcée 8 s après le décollage
  Gunther.setLandingTime(3000);     // au sol = vitesse < 1 m/s pendant 3 s

  if (Gunther.selfTest() != GUNTHER_OK) {
    while (true) { Gunther.ledToggle(4); delay(200); }
  }
  Gunther.logBegin();
  Gunther.led(1, true);
}

void loop() {
  Gunther.update();
  Gunther.logFlight();

  if (Gunther.apogee()) Gunther.servoOpen(1);

  if (Gunther.landed()) {
    Gunther.logText("atterrissage");
    Gunther.logClose();
    Gunther.servoDetach(1);         // le servo n'est plus alimenté en signal
    Serial.print("Altitude max : ");
    Serial.println(Gunther.maxAltitude());
    while (true) {
      Gunther.beepAltitude();       // altitude max par défaut
      delay(5000);
    }
  }
  delay(10);
}
