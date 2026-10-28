// Mode classique : lecture directe de la centrale BMI088 sur le bus SPI1.
// Accéléromètre : CS = BMI088_ACCEL_CS (envoie un octet "vide" avant les données)
// Gyroscope     : CS = BMI088_GYRO_CS

#include <SPI.h>

SPISettings reglages(8000000, MSBFIRST, SPI_MODE0);

uint8_t lireAccel(uint8_t reg, uint8_t *buf = nullptr, uint8_t n = 1) {
  uint8_t v = 0;
  SPI1.beginTransaction(reglages);
  digitalWrite(BMI088_ACCEL_CS, LOW);
  SPI1.transfer(reg | 0x80);
  SPI1.transfer(0);                 // octet vide de l'accéléromètre
  for (uint8_t i = 0; i < n; i++) { v = SPI1.transfer(0); if (buf) buf[i] = v; }
  digitalWrite(BMI088_ACCEL_CS, HIGH);
  SPI1.endTransaction();
  return v;
}

uint8_t lireGyro(uint8_t reg, uint8_t *buf = nullptr, uint8_t n = 1) {
  uint8_t v = 0;
  SPI1.beginTransaction(reglages);
  digitalWrite(BMI088_GYRO_CS, LOW);
  SPI1.transfer(reg | 0x80);
  for (uint8_t i = 0; i < n; i++) { v = SPI1.transfer(0); if (buf) buf[i] = v; }
  digitalWrite(BMI088_GYRO_CS, HIGH);
  SPI1.endTransaction();
  return v;
}

void ecrire(uint8_t cs, uint8_t reg, uint8_t valeur) {
  SPI1.beginTransaction(reglages);
  digitalWrite(cs, LOW);
  SPI1.transfer(reg & 0x7F);
  SPI1.transfer(valeur);
  digitalWrite(cs, HIGH);
  SPI1.endTransaction();
  delay(2);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}
  pinMode(BMI088_ACCEL_CS, OUTPUT); digitalWrite(BMI088_ACCEL_CS, HIGH);
  pinMode(BMI088_GYRO_CS, OUTPUT);  digitalWrite(BMI088_GYRO_CS, HIGH);
  SPI1.begin();

  lireAccel(0x00);                  // la 1re lecture passe l'accéléromètre en SPI
  Serial.print("ACC_CHIP_ID = 0x"); Serial.println(lireAccel(0x00), HEX);   // 0x1E
  Serial.print("GYR_CHIP_ID = 0x"); Serial.println(lireGyro(0x00), HEX);    // 0x0F

  ecrire(BMI088_ACCEL_CS, 0x7D, 0x04);   // accéléromètre allumé
  delay(5);
  ecrire(BMI088_ACCEL_CS, 0x7C, 0x00);   // mode actif
  ecrire(BMI088_ACCEL_CS, 0x41, 0x03);   // ±24 g
  ecrire(BMI088_ACCEL_CS, 0x40, 0xAA);   // 400 Hz
  ecrire(BMI088_GYRO_CS, 0x0F, 0x00);    // ±2000 °/s
  ecrire(BMI088_GYRO_CS, 0x10, 0x82);    // 1000 Hz
}

void loop() {
  uint8_t d[6];
  lireAccel(0x12, d, 6);
  float ax = (int16_t)(d[1] << 8 | d[0]) * 24.0 * 9.80665 / 32768.0;
  float ay = (int16_t)(d[3] << 8 | d[2]) * 24.0 * 9.80665 / 32768.0;
  float az = (int16_t)(d[5] << 8 | d[4]) * 24.0 * 9.80665 / 32768.0;
  lireGyro(0x02, d, 6);
  float gx = (int16_t)(d[1] << 8 | d[0]) * 2000.0 / 32768.0;
  float gy = (int16_t)(d[3] << 8 | d[2]) * 2000.0 / 32768.0;
  float gz = (int16_t)(d[5] << 8 | d[4]) * 2000.0 / 32768.0;
  Serial.printf("acc %7.2f %7.2f %7.2f m/s2   gyro %8.2f %8.2f %8.2f deg/s\r\n", ax, ay, az, gx, gy, gz);
  delay(100);
}
