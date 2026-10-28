// Mode classique : programme de vol complet SANS la bibliothèque Gunther.
//  - décollage : jack arraché
//  - apogée : l'altitude baisse de 3 m sous le maximum, sécurité à 8 s
//  - enregistrement sur la carte SD
// Baromètre BMP390 / BMP388 lu directement (voir l'exemple 07).

#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <Servo.h>

const uint8_t BARO = 0x76;
double T1, T2, T3, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11;
float pressionSol, altitudeMax = 0;
uint32_t tDecollage = 0;
bool enVol = false, apogee = false;

SdFat sd;
FsFile fichier;
Servo trappe;

void baroEcrire(uint8_t reg, uint8_t v) {
  Wire.beginTransmission(BARO); Wire.write(reg); Wire.write(v); Wire.endTransmission();
}

void baroLire(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(BARO); Wire.write(reg); Wire.endTransmission(false);
  Wire.requestFrom(BARO, n);
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
}

float pression() {
  uint8_t d[6];
  baroLire(0x04, d, 6);
  double up = (uint32_t)d[2] << 16 | (uint32_t)d[1] << 8 | d[0];
  double ut = (uint32_t)d[5] << 16 | (uint32_t)d[4] << 8 | d[3];
  double a = ut - T1;
  double t = a * T2 + a * a * T3;
  return (P5 + P6 * t + P7 * t * t + P8 * t * t * t)
       + up * (P1 + P2 * t + P3 * t * t + P4 * t * t * t)
       + up * up * (P9 + P10 * t) + up * up * up * P11;
}

void baroDemarrer() {
  uint8_t c[21];
  baroLire(0x31, c, 21);
  T1 = (uint16_t)(c[1] << 8 | c[0]) * 256.0;
  T2 = (uint16_t)(c[3] << 8 | c[2]) / 1073741824.0;
  T3 = (int8_t)c[4] / 281474976710656.0;
  P1 = ((int16_t)(c[6] << 8 | c[5]) - 16384.0) / 1048576.0;
  P2 = ((int16_t)(c[8] << 8 | c[7]) - 16384.0) / 536870912.0;
  P3 = (int8_t)c[9] / 4294967296.0;
  P4 = (int8_t)c[10] / 137438953472.0;
  P5 = (uint16_t)(c[12] << 8 | c[11]) * 8.0;
  P6 = (uint16_t)(c[14] << 8 | c[13]) / 64.0;
  P7 = (int8_t)c[15] / 256.0;
  P8 = (int8_t)c[16] / 32768.0;
  P9 = (int16_t)(c[18] << 8 | c[17]) / 281474976710656.0;
  P10 = (int8_t)c[19] / 281474976710656.0;
  P11 = (int8_t)c[20] / 36893488147419103232.0;
  baroEcrire(0x1C, 0x03); baroEcrire(0x1D, 0x02); baroEcrire(0x1F, 0x04); baroEcrire(0x1B, 0x33);
  delay(100);
}

void setup() {
  Serial.begin(115200);
  pinMode(JACK_1, INPUT_PULLDOWN);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_4, OUTPUT);
  trappe.attach(SERVO_1);
  trappe.write(0);                  // trappe fermée

  Wire.begin();
  Wire.setClock(400000);
  baroDemarrer();
  float somme = 0;
  for (int i = 0; i < 16; i++) { somme += pression(); delay(25); }
  pressionSol = somme / 16;

  if (!sd.begin(SdSpiConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(12), &SPI)) ||
      !fichier.open("vol.csv", O_WRONLY | O_CREAT | O_APPEND)) {
    digitalWrite(LED_4, HIGH);      // LED 4 = problème carte SD
  }
  fichier.println("temps_ms;altitude_m;en_vol;apogee");
  tone(BUZZER, 4000, 300);
  digitalWrite(LED_1, HIGH);        // prête
}

void loop() {
  float altitude = 44330.0 * (1.0 - pow(pression() / pressionSol, 0.190295));

  if (!enVol && digitalRead(JACK_1) == LOW) {   // jack arraché
    enVol = true;
    tDecollage = millis();
  }
  if (enVol && !apogee) {
    if (altitude > altitudeMax) altitudeMax = altitude;
    if (altitudeMax - altitude >= 3.0 || millis() - tDecollage >= 8000) {
      apogee = true;
      trappe.write(90);             // ouverture
    }
  }

  fichier.printf("%lu;%.2f;%d;%d\r\n", millis(), altitude, enVol, apogee);
  static uint32_t dernierSync = 0;
  if (millis() - dernierSync >= 500) { dernierSync = millis(); fichier.sync(); }
  delay(20);
}
