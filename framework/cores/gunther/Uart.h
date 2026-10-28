/*
 * Gunther.ino Arduino core - hardware UART on a SERCOM ("Serial1")
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#ifndef GUNTHER_UART_H
#define GUNTHER_UART_H

#include "HardwareSerial.h"

#define UART_BUFFER_SIZE 256

class Uart : public HardwareSerial {
public:
  /* sercom: 0..5, txPad / rxPad: SERCOM pad numbers, mux: pin function letter */
  Uart(uint8_t sercom, uint8_t txPin, uint8_t rxPin, uint8_t txPad, uint8_t rxPad, uint8_t mux);

  void begin(unsigned long baud) override { begin(baud, SERIAL_8N1); }
  void begin(unsigned long baud, uint16_t config) override;
  void end() override;

  int available() override;
  int availableForWrite() override;
  int peek() override;
  int read() override;
  void flush() override;
  size_t write(uint8_t c) override;
  using Print::write;
  operator bool() override { return _started; }

  void irqHandler();          /* called from the SERCOM interrupt */

private:
  uint8_t _sercom, _tx, _rx, _txPad, _rxPad, _mux;
  volatile bool _started;
  volatile uint16_t _rxHead, _rxTail, _txHead, _txTail;
  uint8_t _rxBuf[UART_BUFFER_SIZE];
  uint8_t _txBuf[UART_BUFFER_SIZE];
};

extern Uart Serial1;

#endif
