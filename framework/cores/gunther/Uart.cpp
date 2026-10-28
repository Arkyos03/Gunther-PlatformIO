/*
 * Gunther.ino Arduino core - hardware UART on a SERCOM
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#include "gunther_internal.h"

/* Serial1 : extension UART, TX = PA22 (SERCOM3 PAD0), RX = PA23 (SERCOM3 PAD1) */
Uart Serial1(3, PIN_SERIAL1_TX, PIN_SERIAL1_RX, 0, 1, MUX_C);

extern "C" {
void SERCOM3_0_Handler(void) { Serial1.irqHandler(); }
void SERCOM3_1_Handler(void) { Serial1.irqHandler(); }
void SERCOM3_2_Handler(void) { Serial1.irqHandler(); }
void SERCOM3_3_Handler(void) { Serial1.irqHandler(); }
}

Uart::Uart(uint8_t sercom, uint8_t txPin, uint8_t rxPin, uint8_t txPad, uint8_t rxPad, uint8_t mux)
  : _sercom(sercom), _tx(txPin), _rx(rxPin), _txPad(txPad), _rxPad(rxPad), _mux(mux),
    _started(false), _rxHead(0), _rxTail(0), _txHead(0), _txTail(0)
{
}

void Uart::begin(unsigned long baud, uint16_t config)
{
  Sercom *s = gunther_sercom(_sercom);
  if (!s || baud == 0) return;
  if (_started) end();

  gunther_sercom_enable_clock(_sercom);
  s->USART.CTRLA.reg = SERCOM_USART_CTRLA_SWRST;
  while (s->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_SWRST) { }

  uint32_t parity = config & SERIAL_PARITY_MASK;
  uint32_t bits   = config & SERIAL_DATA_MASK;
  if (bits < 5 || bits > 8) bits = 8;

  s->USART.CTRLA.reg = SERCOM_USART_CTRLA_MODE(1) |          /* internal clock      */
                       SERCOM_USART_CTRLA_DORD |             /* LSB first           */
                       SERCOM_USART_CTRLA_RXPO(_rxPad) |
                       SERCOM_USART_CTRLA_TXPO(0) |                 /* TX on PAD0 */
                       SERCOM_USART_CTRLA_SAMPR(0) |         /* 16x, arithmetic baud */
                       SERCOM_USART_CTRLA_FORM(parity ? 1 : 0);

  /* BAUD = 65536 * (1 - 16 * baud / fref) */
  uint64_t ratio = ((uint64_t)16 * 65536ULL * baud) / VARIANT_GCLK1_FREQ;
  s->USART.BAUD.reg = (uint16_t)(65536ULL - ratio);

  s->USART.CTRLB.reg = SERCOM_USART_CTRLB_CHSIZE(bits == 8 ? 0 : bits) |
                       ((config & SERIAL_STOP_MASK) == SERIAL_STOP_2 ? SERCOM_USART_CTRLB_SBMODE : 0) |
                       (parity == SERIAL_PARITY_ODD ? SERCOM_USART_CTRLB_PMODE : 0) |
                       SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;
  while (s->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_CTRLB) { }

  gunther_pin_set_mux(_tx, _mux);
  gunther_pin_set_mux(_rx, _mux);
  PORT->Group[g_pins[_rx].port].PINCFG[g_pins[_rx].pin].reg |= PORT_PINCFG_PULLEN | PORT_PINCFG_INEN;
  PORT->Group[g_pins[_rx].port].OUTSET.reg = 1UL << g_pins[_rx].pin;   /* idle high */

  _rxHead = _rxTail = _txHead = _txTail = 0;
  s->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC | SERCOM_USART_INTENSET_ERROR;

  uint32_t irq = gunther_sercom_irq_base(_sercom);
  for (uint32_t i = 0; i < 4; i++) {
    NVIC_SetPriority((IRQn_Type)(irq + i), 2);
    NVIC_ClearPendingIRQ((IRQn_Type)(irq + i));
    NVIC_EnableIRQ((IRQn_Type)(irq + i));
  }

  s->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
  while (s->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE) { }
  _started = true;
}

void Uart::end()
{
  if (!_started) return;
  flush();
  Sercom *s = gunther_sercom(_sercom);
  uint32_t irq = gunther_sercom_irq_base(_sercom);
  for (uint32_t i = 0; i < 4; i++) NVIC_DisableIRQ((IRQn_Type)(irq + i));
  s->USART.CTRLA.reg &= ~SERCOM_USART_CTRLA_ENABLE;
  while (s->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE) { }
  gunther_pin_set_gpio(_tx);
  gunther_pin_set_gpio(_rx);
  _started = false;
}

int Uart::available()
{
  return (int)((uint16_t)(_rxHead - _rxTail + UART_BUFFER_SIZE) % UART_BUFFER_SIZE);
}

int Uart::availableForWrite()
{
  return (int)(UART_BUFFER_SIZE - 1 - ((uint16_t)(_txHead - _txTail + UART_BUFFER_SIZE) % UART_BUFFER_SIZE));
}

int Uart::peek()
{
  if (_rxHead == _rxTail) return -1;
  return _rxBuf[_rxTail];
}

int Uart::read()
{
  if (_rxHead == _rxTail) return -1;
  uint8_t c = _rxBuf[_rxTail];
  _rxTail = (uint16_t)((_rxTail + 1) % UART_BUFFER_SIZE);
  return c;
}

size_t Uart::write(uint8_t c)
{
  if (!_started) return 0;
  Sercom *s = gunther_sercom(_sercom);

  /* Fast path: buffer empty and data register free */
  if (_txHead == _txTail && (s->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE)) {
    s->USART.DATA.reg = c;
    return 1;
  }

  uint16_t next = (uint16_t)((_txHead + 1) % UART_BUFFER_SIZE);
  while (next == _txTail) {
    /* Buffer full: if interrupts are disabled, push one byte by hand */
    if (__get_PRIMASK() && (s->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE)) {
      s->USART.DATA.reg = _txBuf[_txTail];
      _txTail = (uint16_t)((_txTail + 1) % UART_BUFFER_SIZE);
    }
  }
  _txBuf[_txHead] = c;
  _txHead = next;
  s->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE;
  return 1;
}

void Uart::flush()
{
  if (!_started) return;
  Sercom *s = gunther_sercom(_sercom);
  while (_txHead != _txTail) { }
  /* wait until the last byte has left the shift register */
  s->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;
  if (s->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE) {
    uint32_t t = 0;
    while (!(s->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC) && t++ < 200000UL) { }
  }
}

void Uart::irqHandler()
{
  Sercom *s = gunther_sercom(_sercom);
  uint8_t flags = s->USART.INTFLAG.reg;

  if (flags & SERCOM_USART_INTFLAG_ERROR) {
    s->USART.STATUS.reg = SERCOM_USART_STATUS_BUFOVF | SERCOM_USART_STATUS_FERR | SERCOM_USART_STATUS_PERR;
    s->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_ERROR;
  }

  while (s->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC) {
    uint8_t c = (uint8_t)s->USART.DATA.reg;
    uint16_t next = (uint16_t)((_rxHead + 1) % UART_BUFFER_SIZE);
    if (next != _rxTail) {           /* drop the byte if the buffer is full */
      _rxBuf[_rxHead] = c;
      _rxHead = next;
    }
  }

  if ((s->USART.INTENSET.reg & SERCOM_USART_INTENSET_DRE) && (flags & SERCOM_USART_INTFLAG_DRE)) {
    if (_txHead != _txTail) {
      s->USART.DATA.reg = _txBuf[_txTail];
      _txTail = (uint16_t)((_txTail + 1) % UART_BUFFER_SIZE);
    } else {
      s->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
    }
  }
}
