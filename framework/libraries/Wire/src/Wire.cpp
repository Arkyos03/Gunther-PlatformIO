/*
 * Gunther.ino - Wire (I2C master and slave) library
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "Wire.h"
#include "gunther_internal.h"

TwoWire Wire(5, PIN_WIRE_SDA, PIN_WIRE_SCL, MUX_D);

extern "C" {
void SERCOM5_0_Handler(void) { Wire.onService(); }
void SERCOM5_1_Handler(void) { Wire.onService(); }
void SERCOM5_2_Handler(void) { Wire.onService(); }
void SERCOM5_3_Handler(void) { Wire.onService(); }
}

#define I2C_RISE_TIME_NS 125u

TwoWire::TwoWire(uint8_t sercom, uint8_t sda, uint8_t scl, uint8_t mux)
  : _sercom(sercom), _sda(sda), _scl(scl), _mux(mux), _clock(100000UL),
    _timeoutUs(25000UL), _resetOnTimeout(true), _timeoutFlag(false), _started(false),
    _transmitting(false), _txAddr(0), _txLen(0), _rxLen(0), _rxPos(0)
{
}

static inline void i2c_sync(Sercom *s)
{
  while (s->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SYSOP) { }
}

static void wire_irq(uint8_t sercom, bool on)
{
  uint32_t irq = gunther_sercom_irq_base(sercom);
  for (uint32_t i = 0; i < 4; i++) {
    if (on) {
      NVIC_SetPriority((IRQn_Type)(irq + i), 2);
      NVIC_ClearPendingIRQ((IRQn_Type)(irq + i));
      NVIC_EnableIRQ((IRQn_Type)(irq + i));
    } else {
      NVIC_DisableIRQ((IRQn_Type)(irq + i));
    }
  }
}

void TwoWire::configure()
{
  Sercom *s = gunther_sercom(_sercom);
  gunther_sercom_enable_clock(_sercom);
  wire_irq(_sercom, false);

  if (_slave) {
    s->I2CS.CTRLA.reg = SERCOM_I2CS_CTRLA_SWRST;
    while (s->I2CS.SYNCBUSY.reg & SERCOM_I2CS_SYNCBUSY_SWRST) { }
    s->I2CS.CTRLA.reg = SERCOM_I2CS_CTRLA_MODE(4) |          /* I2C slave */
                        SERCOM_I2CS_CTRLA_SDAHOLD(2) |
                        SERCOM_I2CS_CTRLA_SPEED(_clock > 400000UL ? 1 : 0);
    s->I2CS.CTRLB.reg = 0;
    s->I2CS.ADDR.reg = SERCOM_I2CS_ADDR_ADDR(_slaveAddr);
    s->I2CS.INTENSET.reg = SERCOM_I2CS_INTENSET_PREC | SERCOM_I2CS_INTENSET_AMATCH | SERCOM_I2CS_INTENSET_DRDY;
    s->I2CS.CTRLA.reg |= SERCOM_I2CS_CTRLA_ENABLE;
    while (s->I2CS.SYNCBUSY.reg & SERCOM_I2CS_SYNCBUSY_ENABLE) { }
    gunther_pin_set_mux(_sda, _mux);
    gunther_pin_set_mux(_scl, _mux);
    wire_irq(_sercom, true);
    return;
  }

  s->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_SWRST;
  while (s->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SWRST) { }

  s->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_MODE(5) |          /* I2C master                 */
                      SERCOM_I2CM_CTRLA_SDAHOLD(2) |       /* 300-600 ns SDA hold time   */
                      SERCOM_I2CM_CTRLA_INACTOUT(3) |      /* release a stuck bus        */
                      SERCOM_I2CM_CTRLA_SPEED(_clock > 400000UL ? 1 : 0);
  s->I2CM.CTRLB.reg = 0;
  i2c_sync(s);

  /* fSCL = fGCLK / (10 + 2 * BAUD + fGCLK * Trise) */
  uint32_t f = VARIANT_GCLK1_FREQ;
  int32_t baud = ((int32_t)(f / _clock) - 10 - (int32_t)((uint64_t)f * I2C_RISE_TIME_NS / 1000000000ULL)) / 2;
  if (baud < 1)   baud = 1;
  if (baud > 255) baud = 255;
  s->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD((uint32_t)baud);

  s->I2CM.CTRLA.reg |= SERCOM_I2CM_CTRLA_ENABLE;
  while (s->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_ENABLE) { }

  s->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(1);      /* force IDLE state */
  i2c_sync(s);

  gunther_pin_set_mux(_sda, _mux);
  gunther_pin_set_mux(_scl, _mux);
}

void TwoWire::begin()
{
  _slave = false;
  configure();
  _started = true;
}

void TwoWire::begin(uint8_t address)
{
  _slave = true;
  _slaveAddr = address & 0x7Fu;
  _rxLen = _rxPos = 0;
  _txLen = _txPos = 0;
  configure();
  _started = true;
}

/* Slave mode: called by the SERCOM interrupt */
void TwoWire::onService()
{
  if (!_slave) return;
  Sercom *s = gunther_sercom(_sercom);
  uint8_t flags = s->I2CS.INTFLAG.reg;
  bool masterReads = (s->I2CS.STATUS.reg & SERCOM_I2CS_STATUS_DIR) != 0;

  if (flags & SERCOM_I2CS_INTFLAG_PREC) {                 /* STOP: end of a message */
    s->I2CS.INTFLAG.reg = SERCOM_I2CS_INTFLAG_PREC;
    if (_rxLen && _onReceive) {
      _rxPos = 0;
      _onReceive((int)_rxLen);
    }
    _rxLen = 0;
    return;
  }

  if (flags & SERCOM_I2CS_INTFLAG_AMATCH) {               /* our address */
    if (_rxLen && _onReceive) {                           /* repeated START after a write */
      _rxPos = 0;
      _onReceive((int)_rxLen);
    }
    _rxLen = _rxPos = 0;
    if (masterReads) {
      _txLen = _txPos = 0;
      _transmitting = true;                               /* Wire.write() fills the answer */
      if (_onRequest) _onRequest();
      _transmitting = false;
    }
    s->I2CS.CTRLB.reg = (s->I2CS.CTRLB.reg & ~SERCOM_I2CS_CTRLB_ACKACT) | SERCOM_I2CS_CTRLB_CMD(3);
    return;
  }

  if (flags & SERCOM_I2CS_INTFLAG_DRDY) {
    if (masterReads) {                                    /* send the next byte */
      if (_txPos > 0 && (s->I2CS.STATUS.reg & SERCOM_I2CS_STATUS_RXNACK)) {
        s->I2CS.CTRLB.reg = SERCOM_I2CS_CTRLB_CMD(2);     /* master is done: wait for STOP */
        return;
      }
      s->I2CS.DATA.reg = (_txPos < _txLen) ? _txBuf[_txPos++] : 0xFF;
    } else {                                              /* receive a byte */
      uint8_t c = (uint8_t)s->I2CS.DATA.reg;
      if (_rxLen < WIRE_BUFFER_SIZE) {
        _rxBuf[_rxLen++] = c;
        s->I2CS.CTRLB.reg = (s->I2CS.CTRLB.reg & ~SERCOM_I2CS_CTRLB_ACKACT) | SERCOM_I2CS_CTRLB_CMD(3);
      } else {
        s->I2CS.CTRLB.reg = SERCOM_I2CS_CTRLB_ACKACT | SERCOM_I2CS_CTRLB_CMD(3);
      }
    }
  }
}

void TwoWire::end()
{
  if (!_started) return;
  Sercom *s = gunther_sercom(_sercom);
  wire_irq(_sercom, false);
  s->I2CM.CTRLA.reg &= ~SERCOM_I2CM_CTRLA_ENABLE;
  while (s->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_ENABLE) { }
  gunther_pin_set_gpio(_sda);
  gunther_pin_set_gpio(_scl);
  _started = false;
  _slave = false;
}

void TwoWire::setClock(uint32_t hz)
{
  if (hz < 10000UL)   hz = 10000UL;
  if (hz > 1000000UL) hz = 1000000UL;
  _clock = hz;
  if (_started) configure();
}

void TwoWire::setWireTimeout(uint32_t timeout_us, bool reset_on_timeout)
{
  _timeoutUs = timeout_us;
  _resetOnTimeout = reset_on_timeout;
}

/* Free a slave that keeps SDA low (e.g. after a reset in the middle of a transfer) */
void TwoWire::recoverBus()
{
  if (_slave) return;
  Sercom *s = gunther_sercom(_sercom);
  s->I2CM.CTRLA.reg &= ~SERCOM_I2CM_CTRLA_ENABLE;
  while (s->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_ENABLE) { }

  gunther_pin_set_gpio(_sda);
  gunther_pin_set_gpio(_scl);
  pinMode(_sda, INPUT_PULLUP);
  pinMode(_scl, OUTPUT);
  for (int i = 0; i < 9 && digitalRead(_sda) == LOW; i++) {
    digitalWrite(_scl, LOW);  delayMicroseconds(5);
    digitalWrite(_scl, HIGH); delayMicroseconds(5);
  }
  pinMode(_scl, INPUT_PULLUP);
  configure();
}

bool TwoWire::waitFlag(uint8_t mask)
{
  Sercom *s = gunther_sercom(_sercom);
  uint32_t start = micros();
  while ((s->I2CM.INTFLAG.reg & mask) == 0) {
    if (_timeoutUs && (micros() - start) > _timeoutUs) {
      _timeoutFlag = true;
      if (_resetOnTimeout) recoverBus();
      return false;
    }
  }
  return true;
}

void TwoWire::stop()
{
  Sercom *s = gunther_sercom(_sercom);
  s->I2CM.CTRLB.reg = (s->I2CM.CTRLB.reg & ~SERCOM_I2CM_CTRLB_CMD_Msk) | SERCOM_I2CM_CTRLB_CMD(3);
  i2c_sync(s);
}

void TwoWire::beginTransmission(uint8_t address)
{
  _txAddr = address;
  _txLen = 0;
  _transmitting = true;
}

size_t TwoWire::write(uint8_t data)
{
  if (!_transmitting || _txLen >= WIRE_BUFFER_SIZE) return 0;
  _txBuf[_txLen++] = data;
  return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  size_t n = 0;
  while (n < quantity && write(data[n])) n++;
  return n;
}

uint8_t TwoWire::endTransmission(bool sendStop)
{
  if (!_transmitting || _slave) return 4;
  _transmitting = false;
  if (!_started) begin();
  Sercom *s = gunther_sercom(_sercom);

  s->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR((uint32_t)_txAddr << 1);
  i2c_sync(s);
  if (!waitFlag(SERCOM_I2CM_INTFLAG_MB)) return 5;
  if (s->I2CM.STATUS.reg & (SERCOM_I2CM_STATUS_ARBLOST | SERCOM_I2CM_STATUS_BUSERR)) return 4;
  if (s->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK) { stop(); return 2; }

  for (size_t i = 0; i < _txLen; i++) {
    s->I2CM.DATA.reg = _txBuf[i];
    i2c_sync(s);
    if (!waitFlag(SERCOM_I2CM_INTFLAG_MB)) return 5;
    if (s->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK) { stop(); return 3; }
  }
  if (sendStop) stop();
  return 0;
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool sendStop)
{
  if (_slave) return 0;
  (void)sendStop;                /* a read always ends with NACK + STOP */
  _rxLen = _rxPos = 0;
  if (quantity == 0) return 0;
  if (quantity > WIRE_BUFFER_SIZE) quantity = WIRE_BUFFER_SIZE;
  if (!_started) begin();
  Sercom *s = gunther_sercom(_sercom);

  s->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;           /* ACK the received bytes */
  i2c_sync(s);
  s->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(((uint32_t)address << 1) | 1u);
  i2c_sync(s);

  for (size_t i = 0; i < quantity; i++) {
    if (!waitFlag(SERCOM_I2CM_INTFLAG_SB | SERCOM_I2CM_INTFLAG_MB)) return 0;
    if (s->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB) {      /* address NACK or bus error */
      stop();
      return 0;
    }
    uint8_t value = (uint8_t)s->I2CM.DATA.reg;
    i2c_sync(s);
    _rxBuf[_rxLen++] = value;

    uint32_t ctrlb = s->I2CM.CTRLB.reg & ~(SERCOM_I2CM_CTRLB_CMD_Msk | SERCOM_I2CM_CTRLB_ACKACT);
    if (i + 1 < quantity) ctrlb |= SERCOM_I2CM_CTRLB_CMD(2);                           /* ACK, read next */
    else                  ctrlb |= SERCOM_I2CM_CTRLB_ACKACT | SERCOM_I2CM_CTRLB_CMD(3); /* NACK, STOP     */
    s->I2CM.CTRLB.reg = ctrlb;
    i2c_sync(s);
  }
  return _rxLen;
}
