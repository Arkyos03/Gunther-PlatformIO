// Mode classique : lecture directe du baromètre BMP390 / BMP388 en I2C (Wire),
// avec la compensation de la datasheet Bosch. Adresse 0x76.
// (Pour un SPL07 vendu comme "BMP388", utiliser la bibliothèque Gunther qui le reconnaît.)

#include <Wire.h>

const uint8_t ADRESSE = 0x76;
double T1, T2, T3, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11;
float pressionSol;

void ecrire(uint8_t reg, uint8_t valeur) {
  Wire.beginTransmission(ADRESSE);
  Wire.write(reg);
  Wire.write(valeur);
  Wire.endTransmission();
}

void lire(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(ADRESSE);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADRESSE, n);
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
}

void lireCapteur(float &pression, float &temperature) {
  uint8_t d[6];
  lire(0x04, d, 6);
  double up = (uint32_t)d[2] << 16 | (uint32_t)d[1] << 8 | d[0];
  double ut = (uint32_t)d[5] << 16 | (uint32_t)d[4] << 8 | d[3];
  double a = ut - T1;
  double t = a * T2 + a * a * T3;
  double p = (P5 + P6 * t + P7 * t * t + P8 * t * t * t)
           + up * (P1 + P2 * t + P3 * t * t + P4 * t * t * t)
           + up * up * (P9 + P10 * t) + up * up * up * P11;
  pression = p;
  temperature = t;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}
  Wire.begin();
  Wire.setClock(400000);

  uint8_t id;
  lire(0x00, &id, 1);
  Serial.print("CHIP_ID = 0x");
  Serial.println(id, HEX);          // 0x60 = BMP390, 0x50 = BMP388 / BMP384

  uint8_t c[21];
  lire(0x31, c, 21);                // coefficients de calibration
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

  ecrire(0x1C, 0x03);               // pression x8, température x1
  ecrire(0x1D, 0x02);               // 50 Hz
  ecrire(0x1F, 0x04);               // filtre IIR
  ecrire(0x1B, 0x33);               // mode normal, pression + température
  delay(100);

  float t;
  lireCapteur(pressionSol, t);
}

void loop() {
  float p, t;
  lireCapteur(p, t);
  float altitude = 44330.0 * (1.0 - pow(p / pressionSol, 0.190295));
  Serial.print("pression = ");
  Serial.print(p, 1);
  Serial.print(" Pa   temperature = ");
  Serial.print(t, 2);
  Serial.print(" C   altitude = ");
  Serial.print(altitude, 2);
  Serial.println(" m");
  delay(100);
}
