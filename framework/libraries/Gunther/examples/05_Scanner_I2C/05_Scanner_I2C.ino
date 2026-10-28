// Cherche tous les capteurs branchés sur le bus I2C
// (le BMP390 de la carte répond à l'adresse 0x76)

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  while (!Serial && millis() < 3000) { }   // attend l'ouverture du moniteur série
}

void loop() {
  Serial.println("Recherche I2C...");
  int trouves = 0;
  for (uint8_t adresse = 1; adresse < 127; adresse++) {
    Wire.beginTransmission(adresse);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  capteur trouve a l'adresse 0x%02X\n", adresse);
      trouves++;
    }
  }
  Serial.printf("%d capteur(s)\n\n", trouves);
  delay(3000);
}
