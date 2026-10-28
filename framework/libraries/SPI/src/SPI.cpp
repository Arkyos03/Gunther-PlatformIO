/*
 * Gunther.ino - SPI library
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "SPI.h"
#include "gunther_internal.h"

SPIClass SPI (2, PIN_SPI_MOSI,  PIN_SPI_SCK,  PIN_SPI_MISO,  MUX_C);
SPIClass SPI1(4, PIN_SPI1_MOSI, PIN_SPI1_SCK, PIN_SPI1_MISO, MUX_C);

SPIClass::SPIClass(uint8_t sercom, uint8_t mosi, uint8_t sck, uint8_t miso, uint8_t mux)
  : _sercom(sercom), _mosi(mosi), _sck(sck), _miso(miso), _mux(mux), _started(false)
{
}

void SPIClass::begin()
{
  if (_started) return;
  gunther_sercom_enable_clock(_sercom);
  gunther_pin_set_mux(_mosi, _mux);
  gunther_pin_set_mux(_sck, _mux);
  gunther_pin_set_mux(_miso, _mux);
  _started = true;
  _settings = SPISettings();
  apply(_settings);
}

void SPIClass::end()
{
  if (!_started) return;
  Sercom *s = gunther_sercom(_sercom);
  s->SPI.CTRLA.reg &= ~SERCOM_SPI_CTRLA_ENABLE;
  while (s->SPI.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_ENABLE) { }
  gunther_pin_set_gpio(_mosi);
  gunther_pin_set_gpio(_sck);
  gunther_pin_set_gpio(_miso);
  _started = false;
}

void SPIClass::apply(const SPISettings &st)
{
  Sercom *s = gunther_sercom(_sercom);

  s->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
  while (s->SPI.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_SWRST) { }

  uint32_t ctrla = SERCOM_SPI_CTRLA_MODE(3) |      /* SPI master                 */
                   SERCOM_SPI_CTRLA_DOPO(0) |      /* MOSI = PAD0, SCK = PAD1    */
                   SERCOM_SPI_CTRLA_DIPO(2);       /* MISO = PAD2                */
  if (st.bitOrder == LSBFIRST)  ctrla |= SERCOM_SPI_CTRLA_DORD;
  if (st.dataMode & 0x02)       ctrla |= SERCOM_SPI_CTRLA_CPOL;
  if (st.dataMode & 0x01)       ctrla |= SERCOM_SPI_CTRLA_CPHA;
  s->SPI.CTRLA.reg = ctrla;

  s->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;
  while (s->SPI.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_CTRLB) { }

  /* fSCK = 48 MHz / (2 * (BAUD + 1)), rounded to the closest frequency below the request */
  uint32_t hz = st.clock;
  if (hz > SPI_MAX_CLOCK) hz = SPI_MAX_CLOCK;
  if (hz < 100000UL) hz = 100000UL;
  uint32_t baud = (VARIANT_GCLK1_FREQ + 2 * hz - 1) / (2 * hz) - 1;
  if (baud > 255) baud = 255;
  s->SPI.BAUD.reg = (uint8_t)baud;

  s->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;
  while (s->SPI.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_ENABLE) { }
}

void SPIClass::beginTransaction(SPISettings settings)
{
  if (!_started) begin();
  if (settings != _settings) {
    _settings = settings;
    apply(_settings);
  }
}

uint8_t SPIClass::transfer(uint8_t data)
{
  Sercom *s = gunther_sercom(_sercom);
  while ((s->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_DRE) == 0) { }
  s->SPI.DATA.reg = data;
  while ((s->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_RXC) == 0) { }
  return (uint8_t)s->SPI.DATA.reg;
}

uint16_t SPIClass::transfer16(uint16_t data)
{
  uint8_t hi, lo;
  if (_settings.bitOrder == LSBFIRST) {
    lo = transfer((uint8_t)data);
    hi = transfer((uint8_t)(data >> 8));
  } else {
    hi = transfer((uint8_t)(data >> 8));
    lo = transfer((uint8_t)data);
  }
  return (uint16_t)((hi << 8) | lo);
}

void SPIClass::transfer(void *buf, size_t count)
{
  uint8_t *p = (uint8_t *)buf;
  while (count--) { *p = transfer(*p); p++; }
}

void SPIClass::transfer(const void *txbuf, void *rxbuf, size_t count)
{
  const uint8_t *tx = (const uint8_t *)txbuf;
  uint8_t *rx = (uint8_t *)rxbuf;
  while (count--) {
    uint8_t v = transfer(tx ? *tx++ : 0xFF);
    if (rx) *rx++ = v;
  }
}

void SPIClass::setBitOrder(BitOrder order)
{
  SPISettings s = _settings; s.bitOrder = order; beginTransaction(s);
}

void SPIClass::setDataMode(uint8_t mode)
{
  SPISettings s = _settings; s.dataMode = mode; beginTransaction(s);
}

void SPIClass::setClockDivider(uint8_t div)
{
  if (div == 0) div = 1;
  setFrequency(VARIANT_GCLK1_FREQ / 2 / div);
}

void SPIClass::setFrequency(uint32_t hz)
{
  SPISettings s = _settings; s.clock = hz; beginTransaction(s);
}
