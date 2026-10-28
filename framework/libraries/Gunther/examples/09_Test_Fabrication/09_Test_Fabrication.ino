// =====================================================================
//  Gunther.ino : test complet de la carte avant de la donner aux étudiants
// =====================================================================
//
//  Matériel pour le test :
//   - la carte alimentée par une pile 9 V sur BAT + le câble USB-C
//   - une carte micro SD (FAT32 ou exFAT) dans le lecteur
//   - 4 LED (une sur chaque bornier LED, patte longue sur +)
//   - un fil pour relier les bornes des switchs et du jack
//   - un servo (on le passe de M1 à M4)
//   - 2 petits fils pour les tests UART (Tx-Rx) et SPI (Mosi-Miso)
//
//  Ouvrir le moniteur série à 115200 bauds et suivre les instructions.
//  Réponses : o = oui, n = non, s = passer le test.
//  À la fin : résumé de tous les tests, 3 bips si la carte est bonne.
// =====================================================================

#include <Gunther.h>
#include <SPI.h>
#include <Wire.h>

enum Result { PASS, FAIL, SKIP };

struct Test {
  const char *name;
  Result result;
};

Test tests[40];
int nbTests = 0;

// ------------------------------------------------------------------ outils

void record(const char *name, Result r, const char *detail = "") {
  if (nbTests >= 40) return;
  tests[nbTests].name = name;
  tests[nbTests].result = r;
  nbTests++;
  const char *txt = (r == PASS) ? "OK" : (r == FAIL) ? "ECHEC" : "passe";
  Serial.printf("  -> %-24s %s %s\n\n", name, txt, detail);
}

void flushInput() {
  delay(20);
  while (Serial.available()) Serial.read();
}

// attend une réponse o / n / s
char ask(const char *question) {
  flushInput();
  Serial.printf("  %s (o/n/s) ? ", question);
  while (true) {
    if (Serial.available()) {
      char c = tolower(Serial.read());
      if (c == 'o' || c == 'n' || c == 's') {
        Serial.println(c);
        flushInput();
        return c;
      }
    }
    delay(5);
  }
}

Result fromAnswer(char c) {
  return (c == 'o') ? PASS : (c == 'n') ? FAIL : SKIP;
}

// attend qu'une broche prenne un niveau, 15 s max (s = passer)
bool waitPin(uint8_t pin, int level) {
  flushInput();
  uint32_t t0 = millis();
  while (millis() - t0 < 15000) {
    if (digitalRead(pin) == level) {
      delay(30);
      if (digitalRead(pin) == level) return true;
    }
    if (Serial.available() && tolower(Serial.read()) == 's') return false;
    delay(5);
  }
  return false;
}

void title(int n, const char *t) {
  Serial.printf("[%d] %s\n", n, t);
}

// ------------------------------------------------------------------ tests

void testBattery() {
  title(1, "Alimentation");
  float v = Gunther.batteryVoltage();
  Serial.printf("  Tension pile mesuree : %.2f V\n", v);
  bool ok = v > 7.0f && v < 13.0f;
  record("Mesure pile", ok ? PASS : FAIL, ok ? "" : "(pile branchee ? 7 a 13 V attendus)");
}

void testBaro() {
  title(2, "Barometre BMP390 / BMP388");
  uint8_t id = Gunther.baroReadRegister(0x00);
  Serial.printf("  Identifiant : 0x%02X (0x60 = BMP390, 0x50 = BMP388)\n", id);
  if (!Gunther.baroOK()) { record("Barometre", FAIL, "(ne repond pas)"); return; }
  float p = Gunther.pressure(), t = Gunther.temperature();
  Serial.printf("  Pression %.0f Pa, temperature %.1f C, altitude %.1f m\n", p, t, Gunther.altitudeSea());
  bool ok = p > 80000 && p < 110000 && t > 0 && t < 50;
  record("Barometre", ok ? PASS : FAIL, ok ? "" : "(valeurs impossibles)");
}

void testImu() {
  title(3, "Centrale inertielle BMI088 (carte posee a plat, immobile)");
  uint8_t ida = Gunther.accelReadRegister(0x00), idg = Gunther.gyroReadRegister(0x00);
  Serial.printf("  Identifiants : accel 0x%02X (0x1E), gyro 0x%02X (0x0F)\n", ida, idg);
  if (!Gunther.imuOK()) { record("Centrale inertielle", FAIL, "(ne repond pas)"); return; }
  float ax, ay, az, gx, gy, gz, g = 0, rot = 0;
  for (int i = 0; i < 20; i++) {           // moyenne sur 200 ms
    Gunther.accel(ax, ay, az);
    Gunther.gyro(gx, gy, gz);
    g += Gunther.gForce();
    rot += fabsf(gx) + fabsf(gy) + fabsf(gz);
    delay(10);
  }
  g /= 20; rot /= 20;
  Serial.printf("  Acceleration totale %.2f g (1 attendu), rotation %.2f deg/s (0 attendu)\n", g, rot);
  record("Accelerometre", (g > 0.85f && g < 1.15f) ? PASS : FAIL);
  record("Gyroscope", (rot < 6.0f) ? PASS : FAIL);
}

void testSd() {
  title(4, "Carte micro SD");
  bool present = Gunther.sdCardPresent();
  Serial.printf("  Detection de carte : %s\n", present ? "oui" : "non");
  record("Detection SD", present ? PASS : FAIL);
  if (!present) return;
  bool ok = Gunther.logBegin("test", "essai");
  if (ok) {
    Gunther.log(123, 4.56f, "ecriture");
    Gunther.logFlush();
    Serial.printf("  Fichier %s cree\n", Gunther.logFileName());
    Gunther.logClose();
  }
  record("Ecriture SD", ok ? PASS : FAIL, ok ? "(fichier test_XXX.csv a effacer)" : "");
}

void testBuzzer() {
  title(5, "Buzzer");
  Gunther.beepBlocking(2000, 300);
  Gunther.beepBlocking(4000, 300);
  record("Buzzer", fromAnswer(ask("Deux bips entendus")));
}

void testLeds() {
  title(6, "LED RGB de la carte, puis LED (une LED sur chaque bornier)");
  static const uint32_t colors[] = { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF };
  static const char *colorNames[] = { "rouge", "verte", "bleue", "blanche" };
  for (int k = 0; k < 4; k++) {
    Gunther.rgb(colors[k]);
    char q[48];
    snprintf(q, sizeof(q), "La LED RGB de la carte est %s", colorNames[k]);
    char a = ask(q);
    static const char *names[] = { "LED RGB rouge", "LED RGB verte", "LED RGB bleue", "LED RGB blanche" };
    record(names[k], fromAnswer(a));
  }
  Gunther.rgb(0, 0, 0);

  for (int n = 1; n <= 4; n++) {
    Gunther.allLeds(false);
    Gunther.led(n, true);
    char q[40];
    snprintf(q, sizeof(q), "Seule la LED %d est allumee", n);
    char a = ask(q);
    static const char *names[] = { "LED 1", "LED 2", "LED 3", "LED 4" };
    record(names[n - 1], fromAnswer(a));
  }
  Gunther.allLeds(false);
  Serial.println("  Variation de luminosite sur LED 1 et LED 2...");
  for (int k = 0; k < 2; k++) {
    for (int i = 0; i <= 255; i += 5) { Gunther.ledBrightness(1, i); Gunther.ledBrightness(2, i); delay(8); }
    for (int i = 255; i >= 0; i -= 5) { Gunther.ledBrightness(1, i); Gunther.ledBrightness(2, i); delay(8); }
  }
  record("PWM LED 1 et 2", fromAnswer(ask("LED 1 et 2 ont varie doucement")));
  Gunther.allLeds(false);
}

void testSwitches() {
  title(7, "Switchs (relier + et - de chaque bornier SW avec un fil, s = passer)");
  static const uint8_t pins[] = { SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4 };
  static const char *names[] = { "SW 1", "SW 2", "SW 3", "SW 4" };
  for (int n = 0; n < 4; n++) {
    if (digitalRead(pins[n]) == LOW) Serial.printf("  Attention : %s est deja ferme, enlever le fil\n", names[n]);
    Serial.printf("  Fermer %s...\n", names[n]);
    bool closed = waitPin(pins[n], LOW);
    bool opened = closed && (Serial.println("  OK, ouvrir maintenant..."), waitPin(pins[n], HIGH));
    record(names[n], (closed && opened) ? PASS : FAIL);
  }
}

void testJack() {
  title(8, "Jack (relier + et S du bornier JACK, s = passer)");
  Serial.println("  Brancher le jack (ou le fil)...");
  bool in = waitPin(JACK_1, HIGH);
  bool out = in && (Serial.println("  OK, l'arracher maintenant..."), waitPin(JACK_1, LOW));
  record("Jack", (in && out) ? PASS : FAIL);
}

void testServos() {
  title(9, "Servos (brancher le servo sur M1, puis M2, M3, M4)");
  static const char *names[] = { "Servo M1", "Servo M2", "Servo M3", "Servo M4" };
  for (int n = 1; n <= 4; n++) {
    Serial.printf("  Servo sur M%d : il va bouger 0 -> 90 -> 0 degres\n", n);
    if (ask("Servo branche, on lance") != 'o') { record(names[n - 1], SKIP); continue; }
    for (int k = 0; k < 2; k++) {
      Gunther.servoWrite(n, 0);  delay(600);
      Gunther.servoWrite(n, 90); delay(600);
    }
    Gunther.servoWrite(n, 0);
    record(names[n - 1], fromAnswer(ask("Le servo a bouge")));
  }
}

void testUart() {
  title(10, "UART (relier Tx et Rx avec un fil)");
  if (ask("Fil Tx-Rx en place") != 'o') { record("UART", SKIP); return; }
  Serial1.begin(115200);
  while (Serial1.available()) Serial1.read();
  const char *msg = "Gunther.ino 0123456789";
  Serial1.print(msg);
  Serial1.flush();
  delay(20);
  char buf[32] = { 0 };
  int n = 0;
  while (Serial1.available() && n < 31) buf[n++] = Serial1.read();
  Serial1.end();
  bool ok = strcmp(buf, msg) == 0;
  Serial.printf("  Recu : \"%s\"\n", buf);
  record("UART", ok ? PASS : FAIL);
}

void testSpi() {
  title(11, "SPI externe (relier Mosi et Miso avec un fil)");
  if (ask("Fil Mosi-Miso en place") != 'o') { record("SPI externe", SKIP); return; }
  // tous les CS a l'etat haut : le BMI088 laisse MISO libre
  digitalWrite(BMI088_ACCEL_CS, HIGH);
  digitalWrite(BMI088_GYRO_CS, HIGH);
  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_CS_EXT, LOW);
  bool csLow = digitalRead(SPI_CS_EXT) == LOW;
  bool ok = true;
  const uint8_t pattern[] = { 0x00, 0xFF, 0xA5, 0x5A, 0x3C };
  for (uint8_t b : pattern) if (SPI1.transfer(b) != b) ok = false;
  digitalWrite(SPI_CS_EXT, HIGH);
  bool csHigh = digitalRead(SPI_CS_EXT) == HIGH;
  SPI1.endTransaction();
  record("SPI Mosi/Miso", ok ? PASS : FAIL);
  record("SPI Cs", (csLow && csHigh) ? PASS : FAIL);
  Serial.println("  Enlever le fil Mosi-Miso.");
  delay(300);
}

void testI2c() {
  title(12, "Bus I2C");
  int found = 0;
  bool baro = false;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Peripherique a l'adresse 0x%02X\n", a);
      found++;
      if (a == 0x76) baro = true;
    }
  }
  record("Bus I2C", baro ? PASS : FAIL, baro ? "" : "(BMP390 absent du bus)");
}

// ------------------------------------------------------------------ programme

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }          // attend l'ouverture du moniteur série
  delay(300);

  Serial.println();
  Serial.println("==============================================");
  Serial.println("   Gunther.ino : test de fabrication de la carte");
  Serial.println("==============================================");
  Serial.println("Reponses : o = oui, n = non, s = passer\n");

  Gunther.begin();

  testBattery();
  testBaro();
  testImu();
  testSd();
  testBuzzer();
  testLeds();
  testSwitches();
  testJack();
  testServos();
  testUart();
  testSpi();
  testI2c();

  // ---------------------------------------------------------------- resume
  int nbPass = 0, nbFail = 0, nbSkip = 0;
  Serial.println("==============================================");
  Serial.println("   RESUME");
  Serial.println("==============================================");
  for (int i = 0; i < nbTests; i++) {
    const char *txt = (tests[i].result == PASS) ? "OK" : (tests[i].result == FAIL) ? "ECHEC" : "passe";
    Serial.printf("  %-24s %s\n", tests[i].name, txt);
    if (tests[i].result == PASS) nbPass++;
    else if (tests[i].result == FAIL) nbFail++;
    else nbSkip++;
  }
  Serial.printf("\n  %d OK, %d ECHEC, %d passe(s)\n", nbPass, nbFail, nbSkip);

  if (nbFail == 0) {
    Serial.println("\n  => CARTE BONNE");
    Gunther.led(1, true);
    Gunther.beepCount(3, 4000);
  } else {
    Serial.println("\n  => CARTE A VERIFIER");
    Gunther.beepBlocking(1500, 1500);
  }
  Serial.println("\nAppuyer sur RESET pour recommencer.");
}

void loop() {
  delay(100);
}
