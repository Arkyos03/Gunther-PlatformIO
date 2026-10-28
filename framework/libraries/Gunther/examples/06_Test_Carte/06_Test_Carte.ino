// Test complet de la carte : capteurs, carte SD, pile, puis LEDs, switchs et jack en continu

#include <Gunther.h>

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) { }   // attend l'ouverture du moniteur série
  Gunther.begin();
  Gunther.selfTest(true, false);           // carte SD testée, pile non obligatoire (USB)
}

void loop() {
  static uint8_t led = 1;
  Gunther.allLeds(false);
  Gunther.led(led, true);
  led = (led % 4) + 1;

  Serial.printf("Pile %.2f V | SD %s | SW %d%d%d%d | JACK %d | alt %.2f m | %.2f g\n",
                Gunther.batteryVoltage(),
                Gunther.sdCardPresent() ? "oui" : "non",
                Gunther.switchClosed(1), Gunther.switchClosed(2), Gunther.switchClosed(3), Gunther.switchClosed(4),
                Gunther.jackPlugged(1), Gunther.altitude(), Gunther.gForce());
  delay(500);
}
