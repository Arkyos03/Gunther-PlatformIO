/*
 * Gunther.ino (AIR ESIEA) - board definition (pin map)
 * MCU : ATSAMD51J20A-AUT (TQFP64), 120 MHz, 1 MB flash, 256 KB RAM
 *
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Pin numbers below are the numbers used in sketches:
 *   pinMode(LED_1, OUTPUT);  digitalWrite(LED_1, HIGH);  ...
 */
#ifndef GUNTHER_VARIANT_H
#define GUNTHER_VARIANT_H

#include <stdint.h>

/* ------------------------------------------------------------------------
 * Clocks
 * --------------------------------------------------------------------- */
#ifndef F_CPU
#define F_CPU               120000000UL
#endif
#define VARIANT_MCK         F_CPU
#define VARIANT_GCLK1_FREQ  48000000UL   /* peripheral clock (DFLL48M)   */

/* ------------------------------------------------------------------------
 * Pin numbers (Arduino numbering)
 * --------------------------------------------------------------------- */
/* Terminal blocks : LEDs (outputs) */
#define LED_1           0u    /* PB10 - PWM */
#define LED_2           1u    /* PB11 - PWM */
#define LED_3           2u    /* PB23 */
#define LED_4           3u    /* PB22 */

/* Terminal blocks : switches (inputs, interrupt capable) */
#define SWITCH_1        4u    /* PB04 */
#define SWITCH_2        5u    /* PB05 */
#define SWITCH_3        6u    /* PB06 */
#define SWITCH_4        7u    /* PB07 */

/* Terminal block : launch jack (input, interrupt capable)
 * J17 = VCC / signal, 10k pull-down on the board: jack plugged = HIGH, removed = LOW */
#define JACK_1          8u    /* PB08 */

/* Extra chip select on the sensor SPI bus (SPI1), for an external SPI sensor */
#define SPI1_CS_EXT     9u    /* PA11 */
#define SPI_CS_EXT      SPI1_CS_EXT   /* same name as on the schematic */

/* Servo connectors (PWM on TCC1, 50 Hz) */
#define SERVO_1         10u   /* PA18 */
#define SERVO_2         11u   /* PA19 */
#define SERVO_3         12u   /* PA20 */
#define SERVO_4         13u   /* PA21 */

/* On-board peripherals */
#define BUZZER          14u   /* PB30 - piezo, use tone() */
#define VBAT_SENSE      15u   /* PB00 - battery voltage / 4 */

/* UART extension (Serial1) */
#define PIN_SERIAL1_TX  16u   /* PA22 - SERCOM3 PAD0 */
#define PIN_SERIAL1_RX  17u   /* PA23 - SERCOM3 PAD1 */

/* I2C (Wire) - BMP390 + external sensors */
#define PIN_WIRE_SDA    18u   /* PB02 - SERCOM5 PAD0 */
#define PIN_WIRE_SCL    19u   /* PB03 - SERCOM5 PAD1 */
#define BMP390_INT      20u   /* PB15 */

/* SPI1 - IMU BMI088 */
#define PIN_SPI1_MOSI   21u   /* PB12 - SERCOM4 PAD0 */
#define PIN_SPI1_SCK    22u   /* PB13 - SERCOM4 PAD1 */
#define PIN_SPI1_MISO   23u   /* PB14 - SERCOM4 PAD2 */
#define BMI088_ACCEL_CS 24u   /* PA16 */
#define BMI088_GYRO_CS  25u   /* PA17 */
#define BMI088_ACCEL_INT 26u  /* PB16 */
#define BMI088_GYRO_INT 27u   /* PB17 */

/* SPI - micro SD card */
#define PIN_SPI_MOSI    28u   /* PA12 - SERCOM2 PAD0 */
#define PIN_SPI_SCK     29u   /* PA13 - SERCOM2 PAD1 */
#define PIN_SPI_MISO    30u   /* PA14 - SERCOM2 PAD2 */
#define SD_CS           31u   /* PA15 */
#define SD_DETECT       32u   /* PA27 - card detect of the SD connector */
/* Rev A: CD1 and CD2 of the connector are both tied to SD_DETECT, the pin always reads HIGH.
 * The libraries detect the card by talking to it instead. Set to 1 on a board where
 * SD_DETECT works (LOW = no card, HIGH = card inserted). */
#define SD_DETECT_WORKS 0

/* USB (used by the core, do not touch) */
#define PIN_USB_DM      33u   /* PA24 */
#define PIN_USB_DP      34u   /* PA25 */

/* Free pins (not routed on Rev A, reserved for an extension header) */
#define PIN_EXT_PA02    35u   /* ADC0 AIN0 / DAC0 */
#define PIN_EXT_PA04    36u
#define PIN_EXT_PA05    37u
#define PIN_EXT_PA06    38u
#define PIN_EXT_PA07    39u
/* Built-in RGB LED SK6805-EC15 (LED_BUILDIN on the schematic), through a 74AHCT1G125 buffer */
#define LED_RGB         40u   /* PA08 */
#define PIN_EXT_PA09    41u
#define PIN_EXT_PA10    42u
#define PIN_EXT_PB09    43u   /* ADC0 AIN3 / ADC1 AIN1 */
#define PIN_EXT_PB01    44u
#define PIN_EXT_PB31    45u

#define NUM_DIGITAL_PINS    46u
#define NUM_ANALOG_INPUTS   6u

/* Analog inputs (analogRead) */
#define A0              VBAT_SENSE
#define A1              SWITCH_1
#define A2              SWITCH_2
#define A3              SWITCH_3
#define A4              SWITCH_4
#define A5              JACK_1
static const uint8_t PIN_A0 = A0;

/* Compatibility names used by examples and libraries */
#define LED_BUILTIN     LED_RGB   /* digitalWrite(LED_BUILTIN, HIGH) = white, rgbLedWrite() for colours */
#define LED_BUILDIN     LED_RGB   /* name used on the schematic */
#define PIN_NEOPIXEL    LED_RGB
#define RGB_BUILTIN     LED_RGB
#define PIN_LED         LED_BUILTIN
#define LED_BUILTIN_LEVEL 64u     /* brightness of digitalWrite(LED_BUILTIN, HIGH), 0..255 */
#define SS              SD_CS
#define MOSI            PIN_SPI_MOSI
#define MISO            PIN_SPI_MISO
#define SCK             PIN_SPI_SCK
#define SDA             PIN_WIRE_SDA
#define SCL             PIN_WIRE_SCL

/* Battery divider: R1 = 30k / R2 = 10k  ->  Vbat = Vpin * 4 */
#define VBAT_DIVIDER_RATIO  4.0f

/* Number of hardware interfaces */
#define SPI_INTERFACES_COUNT  2
#define WIRE_INTERFACES_COUNT 1

#define SERIAL_PORT_MONITOR         Serial
#define SERIAL_PORT_USBVIRTUAL      Serial
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial1

/* ------------------------------------------------------------------------
 * Pin description table (used by the core)
 * --------------------------------------------------------------------- */
#define GUNTHER_PORTA   0u
#define GUNTHER_PORTB   1u

#define NO_ADC      0xFFu
#define ADC_CH(inst, ch)   ((uint8_t)(((inst) << 5) | (ch)))   /* inst: 0 = ADC0, 1 = ADC1 */

#define NO_PWM      0xFFu
#define TCC_WO(inst, wo)   ((uint8_t)(((inst) << 4) | (wo)))   /* TCCn / WO[x]             */

#define NO_EXTINT   0xFFu

/* Peripheral function letters (PMUX) */
#define MUX_A 0u
#define MUX_B 1u
#define MUX_C 2u
#define MUX_D 3u
#define MUX_E 4u
#define MUX_F 5u
#define MUX_G 6u
#define MUX_H 7u

typedef struct {
  uint8_t port;      /* GUNTHER_PORTA / GUNTHER_PORTB                   */
  uint8_t pin;       /* bit number in the port (0..31)          */
  uint8_t adc;       /* ADC_CH(...) or NO_ADC                    */
  uint8_t pwm;       /* TCC_WO(...) or NO_PWM                    */
  uint8_t pwm_mux;   /* PMUX function used for the PWM output    */
  uint8_t extint;    /* EXTINT line 0..15 or NO_EXTINT           */
} PinDescription;

#ifdef __cplusplus
extern "C" {
#endif
extern const PinDescription g_pins[NUM_DIGITAL_PINS];
#ifdef __cplusplus
}
#endif

#endif /* GUNTHER_VARIANT_H */
