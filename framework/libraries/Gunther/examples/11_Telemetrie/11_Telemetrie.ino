// Télémétrie : envoie les données de vol par l'UART d'extension (Serial1),
// par exemple vers un module radio (LoRa, APC220, XBee...) branché sur TX1/RX1.
// Les mêmes lignes s'affichent sur le moniteur série (USB) et dans le traceur série.

#include <Gunther.h>

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);              // vitesse du module radio
  Gunther.begin();
  Gunther.setLogSeparator(',');     // ',' : plus facile à lire dans un tableur ou le traceur
  Gunther.printFlightHeader(Serial1);
}

void loop() {
  static uint32_t last = 0;
  Gunther.update();

  if (millis() - last >= 200) {     // 5 lignes par seconde (une radio lente ne suit pas plus)
    last = millis();
    Gunther.printFlight(Serial1);
    Gunther.printFlight(Serial);
    Serial.print("vitesse verticale (m/s) : ");
    Serial.println(Gunther.verticalSpeed());
  }
}
