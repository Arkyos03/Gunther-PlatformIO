/*
 * Gunther.ino - hardware port layer of the common Gunther library
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * The Gunther library is the same for every Gunther.ino board. What depends on
 * the microcontroller is grouped here, one implementation per platform:
 *   Gunther_Port_SAMD51.cpp   (platforms/samd51)
 *   Gunther_Port_Generic.cpp  (any other Arduino core: STM32duino...)
 *
 * Pin names every board variant must provide (see README):
 *   LED_1..LED_4, SWITCH_1..SWITCH_4, JACK_1, SERVO_1..SERVO_4, BUZZER, VBAT_SENSE,
 *   VBAT_DIVIDER_RATIO, SD_CS, BMI088_ACCEL_CS, BMI088_GYRO_CS
 * Optional: LED_RGB (built-in addressable LED), SD_DETECT + SD_DETECT_WORKS,
 *   GUNTHER_BARO_WIRE, GUNTHER_IMU_SPI, GUNTHER_SD_SPI (buses, default Wire / SPI1 / SPI)
 */
#ifndef GUNTHER_PORT_H
#define GUNTHER_PORT_H

#include <Arduino.h>

#ifndef GUNTHER_BARO_WIRE
#define GUNTHER_BARO_WIRE Wire
#endif
#ifndef GUNTHER_IMU_SPI
#define GUNTHER_IMU_SPI   SPI1
#endif
#ifndef GUNTHER_SD_SPI
#define GUNTHER_SD_SPI    SPI
#endif
#ifndef SD_DETECT_WORKS
#define SD_DETECT_WORKS   0
#endif
#define GUNTHER_NO_LED    0xFF

/* Status LED */
uint8_t gunther_port_builtin_led();                                   /* LED_RGB, or GUNTHER_NO_LED */
void    gunther_port_builtin_rgb(uint8_t r, uint8_t g, uint8_t b);    /* does not take the LED from the animations */
void    gunther_port_rgb(uint8_t pin, uint8_t r, uint8_t g, uint8_t b); /* external WS2812 / SK6812 */
bool    gunther_port_sketch_owns_led();                               /* the sketch drove the built-in LED itself */
void    gunther_port_release_led();
bool    gunther_port_has_pwm(uint8_t pin);
/* Calls fn every 20 ms, even during delay(). Returns false if the platform can not:
 * the library then animates the LED from update(). */
bool    gunther_port_start_tick(void (*fn)(void));

/* Board */
float   gunther_port_battery();                                       /* volts */
void    gunther_port_bootloader();
void    gunther_port_toggle(uint8_t pin);

#endif
