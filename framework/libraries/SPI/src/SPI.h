/*
 * Gunther.ino - SPI library (Arduino compatible API)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 *   SPI  : micro SD card bus      (SERCOM2 - PA12 MOSI, PA13 SCK, PA14 MISO)
 *   SPI1 : IMU BMI088 bus         (SERCOM4 - PB12 MOSI, PB13 SCK, PB14 MISO)
 *
 * Chip selects are normal pins: SD_CS, BMI088_ACCEL_CS, BMI088_GYRO_CS.
 */
#ifndef GUNTHER_SPI_H
#define GUNTHER_SPI_H

#include <Arduino.h>

#define SPI_HAS_TRANSACTION 1
#define SPI_HAS_NOTUSINGINTERRUPT 1

#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define SPI_MODE2 0x02
#define SPI_MODE3 0x03

typedef uint8_t BitOrder;

#define SPI_MAX_CLOCK 24000000UL

class SPISettings {
public:
  SPISettings() : clock(4000000UL), bitOrder(MSBFIRST), dataMode(SPI_MODE0) {}
  SPISettings(uint32_t clock_hz, uint8_t order, uint8_t mode)
    : clock(clock_hz), bitOrder(order), dataMode(mode) {}
  bool operator==(const SPISettings &o) const
  { return clock == o.clock && bitOrder == o.bitOrder && dataMode == o.dataMode; }
  bool operator!=(const SPISettings &o) const { return !(*this == o); }

  uint32_t clock;
  uint8_t  bitOrder;
  uint8_t  dataMode;
};

class SPIClass {
public:
  SPIClass(uint8_t sercom, uint8_t mosi, uint8_t sck, uint8_t miso, uint8_t mux);

  void begin();
  void end();

  void beginTransaction(SPISettings settings);
  void endTransaction(void) {}

  uint8_t  transfer(uint8_t data);
  uint16_t transfer16(uint16_t data);
  void     transfer(void *buf, size_t count);
  void     transfer(const void *txbuf, void *rxbuf, size_t count);

  /* Old style API */
  void setBitOrder(BitOrder order);
  void setDataMode(uint8_t mode);
  void setClockDivider(uint8_t div);     /* divider of 48 MHz / 2 */
  void setFrequency(uint32_t hz);

  void usingInterrupt(int) {}
  void notUsingInterrupt(int) {}
  void attachInterrupt() {}
  void detachInterrupt() {}

private:
  void apply(const SPISettings &s);
  uint8_t _sercom, _mosi, _sck, _miso, _mux;
  bool _started;
  SPISettings _settings;
};

extern SPIClass SPI;    /* SD card */
extern SPIClass SPI1;   /* IMU     */

#endif
