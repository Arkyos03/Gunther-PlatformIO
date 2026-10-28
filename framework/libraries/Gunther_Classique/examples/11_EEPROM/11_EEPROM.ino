// Mode classique : mémoire EEPROM (gardée après une coupure et un nouveau téléversement).
// 4096 octets, même utilisation que sur une Arduino Uno.
// Ne pas écrire en boucle pendant un vol : la mémoire s'use (~100 000 écritures).

#include <EEPROM.h>

struct Reglages {
  float seuilApogee;
  int angleOuvert;
  int angleFerme;
};

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}

  // compteur de démarrages à l'adresse 0
  uint8_t n = EEPROM.read(0);
  if (n == 255) n = 0;              // EEPROM neuve : tout vaut 255
  n++;
  EEPROM.write(0, n);
  Serial.print("demarrage numero ");
  Serial.println(n);

  // une structure complète à l'adresse 10
  Reglages r;
  EEPROM.get(10, r);
  if (isnan(r.seuilApogee)) {       // jamais enregistré : valeurs par défaut
    r = { 3.0f, 90, 0 };
    EEPROM.put(10, r);
  }
  Serial.print("seuil apogee = ");
  Serial.print(r.seuilApogee);
  Serial.print(" m, servo ouvert = ");
  Serial.print(r.angleOuvert);
  Serial.print(", ferme = ");
  Serial.println(r.angleFerme);
}

void loop() {}
