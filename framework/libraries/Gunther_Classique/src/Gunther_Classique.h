/*
 * Gunther.ino - classic mode
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Nothing to include: in classic mode the board is programmed like an Arduino Nano,
 * with the pin names of the board (they are defined by the board itself):
 *
 *   LED_1 .. LED_4         outputs (LED_1, LED_2: analogWrite PWM)
 *   SWITCH_1 .. SWITCH_4   inputs, switch to GND -> pinMode(SWITCH_1, INPUT_PULLUP)
 *   JACK_1                 input, 10k pull-down: plugged = HIGH
 *   SERVO_1 .. SERVO_4     #include <Servo.h>
 *   BUZZER                 tone(BUZZER, 4000, 100)
 *   VBAT_SENSE (A0)        analogRead(), or readBatteryVoltage() in volts
 *   Wire                   I2C: barometer at 0x76, BMP390_INT
 *   SPI1                   BMI088: BMI088_ACCEL_CS, BMI088_GYRO_CS, BMI088_ACCEL_INT,
 *                          BMI088_GYRO_INT, external sensor SPI1_CS_EXT
 *   SPI                    micro SD: SD_CS  (#include <SdFat.h>; SD_DETECT does not work on Rev A)
 *   Serial / Serial1       USB / UART extension (PIN_SERIAL1_TX, PIN_SERIAL1_RX)
 *   EEPROM, Watchdog       #include <EEPROM.h>, #include <Watchdog.h>
 *
 * See File > Examples > Gunther Classique.
 */
#ifndef GUNTHER_CLASSIQUE_H
#define GUNTHER_CLASSIQUE_H
#include <Arduino.h>
#endif
