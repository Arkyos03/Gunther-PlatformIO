// Mode classique : chien de garde (watchdog).
// Si le programme se bloque plus de 2 s, la carte redémarre toute seule.
// Envoyer 'b' dans le moniteur série pour simuler un blocage.

#include <Watchdog.h>

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}
  Serial.print("cause du dernier redemarrage : ");
  Serial.println(Watchdog.resetCauseText());

  uint32_t ms = Watchdog.enable(2000);
  Serial.print("watchdog actif : ");
  Serial.print(ms);
  Serial.println(" ms");
  pinMode(LED_1, OUTPUT);
}

void loop() {
  Watchdog.reset();                 // "je suis vivant"
  digitalToggle(LED_1);
  delay(200);

  if (Serial.read() == 'b') {
    Serial.println("blocage...");
    while (true) {}                 // le watchdog va redémarrer la carte
  }
}
