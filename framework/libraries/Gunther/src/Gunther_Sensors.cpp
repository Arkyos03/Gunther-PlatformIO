/*
 * Gunther.ino - barometer (BMP390 / BMP388 / SPL07) and IMU (BMI088) drivers
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Register maps and compensation formulas from the Bosch BMP390 / BMI088 datasheets
 * and from the Goertek SPL07-003 datasheet (also sold as "BMP388" by Tokmas).
 */
#include "Gunther.h"
#include <Wire.h>
#include <SPI.h>
#include "Gunther_Port.h"

/* ======================================================================
 * BMP390 (I2C, address 0x76)
 * ==================================================================== */
#define BARO_ADDR        0x76
#define BARO_CHIP_ID     0x00
#define BARO_ERR         0x02
#define BARO_DATA        0x04
#define BARO_PWR_CTRL    0x1B
#define BARO_OSR         0x1C
#define BARO_ODR         0x1D
#define BARO_CONFIG      0x1F
#define BARO_CALIB       0x31
#define BARO_CMD         0x7E
#define BARO_PERIOD_MS   20      /* 50 Hz output data rate */

static bool baroWrite(uint8_t reg, uint8_t value)
{
  GUNTHER_BARO_WIRE.beginTransmission(BARO_ADDR);
  GUNTHER_BARO_WIRE.write(reg);
  GUNTHER_BARO_WIRE.write(value);
  return GUNTHER_BARO_WIRE.endTransmission() == 0;
}

static bool baroReadBytes(uint8_t reg, uint8_t *buf, uint8_t len)
{
  GUNTHER_BARO_WIRE.beginTransmission(BARO_ADDR);
  GUNTHER_BARO_WIRE.write(reg);
  if (GUNTHER_BARO_WIRE.endTransmission(false) != 0) return false;
  if (GUNTHER_BARO_WIRE.requestFrom((uint8_t)BARO_ADDR, (size_t)len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = (uint8_t)GUNTHER_BARO_WIRE.read();
  return true;
}

uint8_t GuntherBoard::baroReadRegister(uint8_t reg)
{
  uint8_t v = 0;
  baroReadBytes(reg, &v, 1);
  return v;
}

bool GuntherBoard::baroBegin()
{
  GUNTHER_BARO_WIRE.begin();
  GUNTHER_BARO_WIRE.setClock(400000);
  _baroType = 0;

  /* Bosch: CHIP_ID (0x00) = 0x60 or 0x50.  SPL07 / SPL06: ID (0x0D) = 0x11 or 0x10.
   * If both match by chance (pressure data of an SPL), the Bosch REV_ID (0x01) = 0x01 decides. */
  uint8_t id = baroReadRegister(BARO_CHIP_ID);
  uint8_t splId = baroReadRegister(0x0D);
  bool bosch = (id == 0x60 || id == 0x50);
  bool spl = (splId == 0x11 || splId == 0x10);
  if (bosch && spl) bosch = (baroReadRegister(0x01) == 0x01);
  if (!bosch) return spl ? splBegin() : false;
  _baroType = (id == 0x60) ? 1 : 2;

  baroWrite(BARO_CMD, 0xB6);                            /* soft reset */
  delay(10);

  uint8_t c[21];
  if (!baroReadBytes(BARO_CALIB, c, sizeof(c))) return false;
  uint16_t t1 = (uint16_t)(c[1] << 8 | c[0]);
  uint16_t t2 = (uint16_t)(c[3] << 8 | c[2]);
  int8_t   t3 = (int8_t)c[4];
  int16_t  p1 = (int16_t)(c[6] << 8 | c[5]);
  int16_t  p2 = (int16_t)(c[8] << 8 | c[7]);
  int8_t   p3 = (int8_t)c[9];
  int8_t   p4 = (int8_t)c[10];
  uint16_t p5 = (uint16_t)(c[12] << 8 | c[11]);
  uint16_t p6 = (uint16_t)(c[14] << 8 | c[13]);
  int8_t   p7 = (int8_t)c[15];
  int8_t   p8 = (int8_t)c[16];
  int16_t  p9 = (int16_t)(c[18] << 8 | c[17]);
  int8_t   p10 = (int8_t)c[19];
  int8_t   p11 = (int8_t)c[20];

  _parT1 = (double)t1 * 256.0;                          /* / 2^-8 */
  _parT2 = (double)t2 / 1073741824.0;                   /* / 2^30 */
  _parT3 = (double)t3 / 281474976710656.0;              /* / 2^48 */
  _parP1 = ((double)p1 - 16384.0) / 1048576.0;          /* / 2^20 */
  _parP2 = ((double)p2 - 16384.0) / 536870912.0;        /* / 2^29 */
  _parP3 = (double)p3 / 4294967296.0;                   /* / 2^32 */
  _parP4 = (double)p4 / 137438953472.0;                 /* / 2^37 */
  _parP5 = (double)p5 * 8.0;                            /* / 2^-3 */
  _parP6 = (double)p6 / 64.0;                           /* / 2^6  */
  _parP7 = (double)p7 / 256.0;                          /* / 2^8  */
  _parP8 = (double)p8 / 32768.0;                        /* / 2^15 */
  _parP9 = (double)p9 / 281474976710656.0;              /* / 2^48 */
  _parP10 = (double)p10 / 281474976710656.0;            /* / 2^48 */
  _parP11 = (double)p11 / 36893488147419103232.0;       /* / 2^65 */

  /* pressure x8, temperature x1, 50 Hz, IIR filter coefficient 3, normal mode */
  baroWrite(BARO_OSR, 0x03);
  baroWrite(BARO_ODR, 0x02);
  baroWrite(BARO_CONFIG, 0x02 << 1);
  baroWrite(BARO_PWR_CTRL, 0x33);
  delay(5);
  if (baroReadRegister(BARO_ERR) & 0x04) {              /* configuration error: slower rate */
    baroWrite(BARO_ODR, 0x03);
    baroWrite(BARO_PWR_CTRL, 0x33);
  }
  delay(60);
  _lastBaroSample = 0;
  return baroRead();
}

/* Reads a new sample (at most every 20 ms). Returns true if a new value was read. */
bool GuntherBoard::baroRead()
{
  if (_baroType >= 3) return splRead();
  uint32_t now = millis();
  if (_lastBaroSample != 0 && now - _lastBaroSample < BARO_PERIOD_MS) return false;

  uint8_t d[6];
  if (!baroReadBytes(BARO_DATA, d, 6)) return false;
  _lastBaroSample = now ? now : 1;

  double up = (double)((uint32_t)d[2] << 16 | (uint32_t)d[1] << 8 | d[0]);
  double ut = (double)((uint32_t)d[5] << 16 | (uint32_t)d[4] << 8 | d[3]);

  /* temperature */
  double pd1 = ut - _parT1;
  double pd2 = pd1 * _parT2;
  double t = pd2 + (pd1 * pd1) * _parT3;

  /* pressure */
  double t2 = t * t, t3 = t2 * t;
  double out1 = _parP5 + _parP6 * t + _parP7 * t2 + _parP8 * t3;
  double out2 = up * (_parP1 + _parP2 * t + _parP3 * t2 + _parP4 * t3);
  double up2 = up * up;
  double out3 = up2 * (_parP9 + _parP10 * t) + up2 * up * _parP11;
  double p = out1 + out2 + out3;

  if (p < 10000.0 || p > 120000.0) return false;       /* impossible value: ignore */
  _temperature = (float)t;
  _pressure = (float)p;
  _baroSamples++;
  speedUpdate();
  return true;
}

/* ======================================================================
 * SPL07-003 / SPL06-001 (I2C, address 0x76): same footprint as the BMP388
 * ==================================================================== */
#define SPL_PSR         0x00
#define SPL_PRS_CFG     0x06
#define SPL_TMP_CFG     0x07
#define SPL_MEAS_CFG    0x08
#define SPL_CFG_REG     0x09
#define SPL_RESET       0x0C
#define SPL_ID          0x0D
#define SPL_COEF        0x10
#define SPL_KP          7864320.0f    /* pressure x8 oversampling   */
#define SPL_KT          524288.0f     /* temperature single         */

static int32_t splSigned(uint32_t v, uint8_t bits)
{
  return (v & (1UL << (bits - 1))) ? (int32_t)(v - (1UL << bits)) : (int32_t)v;
}

bool GuntherBoard::splBegin()
{
  uint8_t id = baroReadRegister(SPL_ID);
  if (id != 0x11 && id != 0x10) return false;           /* 0x11 = SPL07-003, 0x10 = SPL06-001 */
  _baroType = (id == 0x11) ? 3 : 4;

  baroWrite(SPL_RESET, 0x09);                           /* soft reset */
  delay(45);
  uint32_t t0 = millis();
  while ((baroReadRegister(SPL_MEAS_CFG) & 0xC0) != 0xC0) {   /* COEF_RDY and SENSOR_RDY */
    if (millis() - t0 > 200) return false;
    delay(5);
  }

  uint8_t c[21] = { 0 };
  if (!baroReadBytes(SPL_COEF, c, _baroType == 3 ? 21 : 18)) return false;
  _splC0  = (float)splSigned(((uint32_t)c[0] << 4) | (c[1] >> 4), 12);
  _splC1  = (float)splSigned(((uint32_t)(c[1] & 0x0F) << 8) | c[2], 12);
  _splC00 = (float)splSigned(((uint32_t)c[3] << 12) | ((uint32_t)c[4] << 4) | (c[5] >> 4), 20);
  _splC10 = (float)splSigned(((uint32_t)(c[5] & 0x0F) << 16) | ((uint32_t)c[6] << 8) | c[7], 20);
  _splC01 = (float)(int16_t)(c[8] << 8 | c[9]);
  _splC11 = (float)(int16_t)(c[10] << 8 | c[11]);
  _splC20 = (float)(int16_t)(c[12] << 8 | c[13]);
  _splC21 = (float)(int16_t)(c[14] << 8 | c[15]);
  _splC30 = (float)(int16_t)(c[16] << 8 | c[17]);
  _splC31 = 0.0f;
  _splC40 = 0.0f;
  if (_baroType == 3) {
    _splC31 = (float)splSigned(((uint32_t)c[18] << 4) | (c[19] >> 4), 12);
    _splC40 = (float)splSigned(((uint32_t)(c[19] & 0x0F) << 8) | c[20], 12);
  }

  /* pressure 32 Hz (SPL06) or 50 Hz (SPL07) x8, temperature 2 Hz x1, continuous */
  baroWrite(SPL_PRS_CFG, _baroType == 3 ? 0xD3 : 0x53);
  baroWrite(SPL_TMP_CFG, 0x10);
  baroWrite(SPL_CFG_REG, 0x00);                         /* no shift needed up to x8 */
  baroWrite(SPL_MEAS_CFG, 0x07);
  delay(60);
  _lastBaroSample = 0;
  return splRead();
}

bool GuntherBoard::splRead()
{
  uint32_t now = millis();
  uint32_t period = (_baroType == 3) ? BARO_PERIOD_MS : 31u;
  if (_lastBaroSample != 0 && now - _lastBaroSample < period) return false;

  uint8_t d[6];
  if (!baroReadBytes(SPL_PSR, d, 6)) return false;
  _lastBaroSample = now ? now : 1;

  float psc = (float)splSigned(((uint32_t)d[0] << 16) | ((uint32_t)d[1] << 8) | d[2], 24) / SPL_KP;
  float tsc = (float)splSigned(((uint32_t)d[3] << 16) | ((uint32_t)d[4] << 8) | d[5], 24) / SPL_KT;

  float t = _splC0 * 0.5f + _splC1 * tsc;
  float p = _splC00 + psc * (_splC10 + psc * (_splC20 + psc * (_splC30 + psc * _splC40)))
          + tsc * (_splC01 + psc * (_splC11 + psc * (_splC21 + psc * _splC31)));

  if (p < 10000.0f || p > 120000.0f) return false;     /* impossible value: ignore */
  _temperature = t;
  _pressure = p;
  _baroSamples++;
  speedUpdate();
  return true;
}

/* The barometer found by begin() still answers */
bool GuntherBoard::baroPresent()
{
  if (_baroType == 1) return baroReadRegister(BARO_CHIP_ID) == 0x60;
  if (_baroType == 2) return baroReadRegister(BARO_CHIP_ID) == 0x50;
  if (_baroType == 3) return baroReadRegister(SPL_ID) == 0x11;
  if (_baroType == 4) return baroReadRegister(SPL_ID) == 0x10;
  return false;
}

const char *GuntherBoard::baroName()
{
  switch (_baroType) {
    case 1:  return "BMP390";
    case 2:  return "BMP388/BMP384";
    case 3:  return "SPL07-003";
    case 4:  return "SPL06-001";
    default: return "aucun";
  }
}

/* Vertical speed from the altitude, low-pass filtered */
void GuntherBoard::speedUpdate()
{
  uint32_t now = millis();
  float alt = 44330.0f * (1.0f - powf(_pressure / _groundPressure, 0.190295f));
  if (_lastAltTime != 0) {
    float dt = (float)(now - _lastAltTime) / 1000.0f;
    if (dt > 0.0f && dt < 1.0f) {
      float v = (alt - _lastAlt) / dt;
      _vSpeed += 0.2f * (v - _vSpeed);
    }
  }
  _lastAlt = alt;
  _lastAltTime = now ? now : 1;
}

float GuntherBoard::verticalSpeed()
{
  update();
  return _vSpeed;
}

void GuntherBoard::setGroundLevel()
{
  if (!_baroOK) return;
  double sum = 0;
  int n = 0;
  uint32_t t0 = millis();
  while (n < 16 && millis() - t0 < 1500) {
    if (baroRead()) { sum += _pressure; n++; }
    delay(1);
  }
  if (n > 0) _groundPressure = (float)(sum / n);
}

float GuntherBoard::pressure()    { update(); return _pressure; }
float GuntherBoard::temperature() { update(); return _temperature; }

float GuntherBoard::altitude()
{
  update();
  return 44330.0f * (1.0f - powf(_pressure / _groundPressure, 0.190295f));
}

float GuntherBoard::altitudeSea(float seaLevel_hPa)
{
  update();
  return 44330.0f * (1.0f - powf(_pressure / (seaLevel_hPa * 100.0f), 0.190295f));
}

/* ======================================================================
 * BMI088 (SPI1)
 * ==================================================================== */
static const SPISettings IMU_SPI(8000000, MSBFIRST, SPI_MODE0);
#define ACC_SCALE   (24.0f * 9.80665f / 32768.0f)    /* ±24 g range -> m/s² */
#define GYR_SCALE   (2000.0f / 32768.0f)             /* ±2000 °/s range     */

static void accRead(uint8_t reg, uint8_t *buf, uint8_t len)
{
  GUNTHER_IMU_SPI.beginTransaction(IMU_SPI);
  digitalWrite(BMI088_ACCEL_CS, LOW);
  GUNTHER_IMU_SPI.transfer(reg | 0x80);
  GUNTHER_IMU_SPI.transfer(0x00);                                  /* accelerometer sends a dummy byte first */
  for (uint8_t i = 0; i < len; i++) buf[i] = GUNTHER_IMU_SPI.transfer(0x00);
  digitalWrite(BMI088_ACCEL_CS, HIGH);
  GUNTHER_IMU_SPI.endTransaction();
}

static void accWrite(uint8_t reg, uint8_t value)
{
  GUNTHER_IMU_SPI.beginTransaction(IMU_SPI);
  digitalWrite(BMI088_ACCEL_CS, LOW);
  GUNTHER_IMU_SPI.transfer(reg & 0x7F);
  GUNTHER_IMU_SPI.transfer(value);
  digitalWrite(BMI088_ACCEL_CS, HIGH);
  GUNTHER_IMU_SPI.endTransaction();
  delay(2);
}

static void gyrRead(uint8_t reg, uint8_t *buf, uint8_t len)
{
  GUNTHER_IMU_SPI.beginTransaction(IMU_SPI);
  digitalWrite(BMI088_GYRO_CS, LOW);
  GUNTHER_IMU_SPI.transfer(reg | 0x80);
  for (uint8_t i = 0; i < len; i++) buf[i] = GUNTHER_IMU_SPI.transfer(0x00);
  digitalWrite(BMI088_GYRO_CS, HIGH);
  GUNTHER_IMU_SPI.endTransaction();
}

static void gyrWrite(uint8_t reg, uint8_t value)
{
  GUNTHER_IMU_SPI.beginTransaction(IMU_SPI);
  digitalWrite(BMI088_GYRO_CS, LOW);
  GUNTHER_IMU_SPI.transfer(reg & 0x7F);
  GUNTHER_IMU_SPI.transfer(value);
  digitalWrite(BMI088_GYRO_CS, HIGH);
  GUNTHER_IMU_SPI.endTransaction();
  delay(2);
}

uint8_t GuntherBoard::accelReadRegister(uint8_t reg) { uint8_t v; accRead(reg, &v, 1); return v; }
uint8_t GuntherBoard::gyroReadRegister(uint8_t reg)  { uint8_t v; gyrRead(reg, &v, 1); return v; }

bool GuntherBoard::imuBegin()
{
  pinMode(BMI088_ACCEL_CS, OUTPUT); digitalWrite(BMI088_ACCEL_CS, HIGH);
  pinMode(BMI088_GYRO_CS, OUTPUT);  digitalWrite(BMI088_GYRO_CS, HIGH);
  GUNTHER_IMU_SPI.begin();
  delay(2);

  /* accelerometer: a first read switches it to SPI mode */
  accelReadRegister(0x00);
  delay(1);
  bool accOK = accelReadRegister(0x00) == 0x1E;
  if (accOK) {
    accWrite(0x7E, 0xB6);                               /* soft reset */
    delay(50);
    accelReadRegister(0x00);                            /* back to SPI mode */
    accWrite(0x7D, 0x04);                               /* accelerometer on */
    delay(5);
    accWrite(0x7C, 0x00);                               /* active mode */
    delay(5);
    accWrite(0x41, 0x03);                               /* ±24 g */
    accWrite(0x40, 0xAA);                               /* 400 Hz, normal filter */
  }

  /* gyroscope */
  bool gyrOK = gyroReadRegister(0x00) == 0x0F;
  if (gyrOK) {
    gyrWrite(0x14, 0xB6);                               /* soft reset */
    delay(50);
    gyrWrite(0x0F, 0x00);                               /* ±2000 °/s */
    gyrWrite(0x10, 0x82);                               /* 1000 Hz, 116 Hz filter */
    gyrWrite(0x11, 0x00);                               /* normal mode */
  }
  return accOK && gyrOK;
}

void GuntherBoard::imuRead()
{
  uint8_t d[6];
  accRead(0x12, d, 6);
  _ax = (int16_t)(d[1] << 8 | d[0]) * ACC_SCALE;
  _ay = (int16_t)(d[3] << 8 | d[2]) * ACC_SCALE;
  _az = (int16_t)(d[5] << 8 | d[4]) * ACC_SCALE;
  gyrRead(0x02, d, 6);
  _gx = (int16_t)(d[1] << 8 | d[0]) * GYR_SCALE;
  _gy = (int16_t)(d[3] << 8 | d[2]) * GYR_SCALE;
  _gz = (int16_t)(d[5] << 8 | d[4]) * GYR_SCALE;
}

void GuntherBoard::accel(float &x, float &y, float &z) { update(); x = _ax; y = _ay; z = _az; }
void GuntherBoard::gyro(float &x, float &y, float &z)  { update(); x = _gx; y = _gy; z = _gz; }
float GuntherBoard::accelX() { update(); return _ax; }
float GuntherBoard::accelY() { update(); return _ay; }
float GuntherBoard::accelZ() { update(); return _az; }
float GuntherBoard::gyroX()  { update(); return _gx; }
float GuntherBoard::gyroY()  { update(); return _gy; }
float GuntherBoard::gyroZ()  { update(); return _gz; }

float GuntherBoard::gForce()
{
  update();
  return sqrtf(_ax * _ax + _ay * _ay + _az * _az) / 9.80665f;
}
