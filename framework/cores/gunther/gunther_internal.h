/*
 * Gunther.ino Arduino core - internal helpers (not part of the public API)
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 */
#ifndef GUNTHER_INTERNAL_H
#define GUNTHER_INTERNAL_H

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GCLK generators set up by SystemInit() */
#define GUNTHER_GCLK_CPU      0   /* 120 MHz (DPLL0)          */
#define GUNTHER_GCLK_48M      1   /* 48 MHz  (DFLL48M, USB)   */
#define GUNTHER_GCLK_32K      3   /* 32.768 kHz (XOSC32K or OSCULP32K) */
#define GUNTHER_GCLK_1M       5   /* 1 MHz, DPLL reference    */

/* Route a GCLK generator to a peripheral channel */
static inline void gunther_gclk_enable(uint32_t channel, uint32_t generator)
{
  GCLK->PCHCTRL[channel].reg = GCLK_PCHCTRL_GEN(generator) | GCLK_PCHCTRL_CHEN;
  while ((GCLK->PCHCTRL[channel].reg & GCLK_PCHCTRL_CHEN) == 0) { }
}

/* Select a peripheral function (MUX_A..MUX_N) on a pin */
void gunther_pin_set_mux(uint32_t pin, uint32_t mux);
/* Same, from raw port/pin numbers */
void gunther_port_set_mux(uint32_t port, uint32_t bit, uint32_t mux);
/* Back to plain GPIO (peripheral disconnected) */
void gunther_pin_set_gpio(uint32_t pin);

/* SERCOM helpers */
Sercom  *gunther_sercom(uint32_t index);
void     gunther_sercom_enable_clock(uint32_t index);
uint32_t gunther_sercom_irq_base(uint32_t index);   /* first of the 4 IRQ numbers */

/* TCC PWM helpers (wiring_analog.c) */
Tcc     *gunther_tcc(uint32_t index);
void     gunther_tcc_init(uint32_t index);
uint32_t gunther_tcc_period(uint32_t index);
void     gunther_tcc_write(uint32_t index, uint32_t cc, uint32_t value);
#define  GUNTHER_SERVO_TCC        1u
#define  GUNTHER_SERVO_TICKS_US   3u       /* TCC1 runs at 3 MHz -> 3 ticks per us  */
#define  GUNTHER_SERVO_PERIOD     60000u   /* 20 ms                                 */

/* RGB LED (rgb_led.c) */
void gunther_rgb_send(uint32_t pin, const uint8_t *grb, uint32_t count);
void gunther_builtin_rgb(uint8_t r, uint8_t g, uint8_t b);     /* does not take the LED from the Gunther library */
void gunther_builtin_digital(uint32_t value);
void gunther_builtin_level(uint8_t level);
int  gunther_builtin_read(void);
extern volatile uint8_t gunther_builtin_user;                   /* 1 once the sketch drives LED_BUILTIN itself */
/* Function called every 20 ms at low priority (PendSV), used for LED animations */
extern void (*volatile gunther_led_hook)(void);

/* USB */
void gunther_usb_init(void);
void gunther_usb_task(void);

/* Analog init */
void gunther_analog_init(void);
int  gunther_analog_read_raw(uint32_t pin);   /* 12-bit, -1 if not analog */
float gunther_analog_ref_volts(void);         /* current analogRead() reference */

#ifdef __cplusplus
}
#endif

#endif
