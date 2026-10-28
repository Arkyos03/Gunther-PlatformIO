// Affiche la tension de la batterie dans le moniteur série

#include <Gunther.h>

void setup() {
  Serial.begin(115200);
  Gunther.begin();
}

void loop() {
  float v = Gunther.batteryVoltage();
  Serial.print("Batterie : ");
  Serial.print(v, 2);
  Serial.println(" V");

  if (v < 7.0) Gunther.beep(2000, 200);   // alerte batterie faible
  delay(1000);
}
