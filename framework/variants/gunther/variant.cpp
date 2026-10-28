/*
 * Gunther.ino (AIR ESIEA) - pin description table
 * Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License (see LICENSE)
 *
 * Pin multiplexing taken from the ATSAMD51J20A datasheet (I/O multiplexing table).
 */
#include "Arduino.h"
#include "gunther_internal.h"

const PinDescription g_pins[NUM_DIGITAL_PINS] = {
  /*      port       pin  adc           pwm           pwm_mux  extint */
  /*  0 LED_1    */ { GUNTHER_PORTB, 10, NO_ADC,       TCC_WO(0, 4), MUX_F, 10 },
  /*  1 LED_2    */ { GUNTHER_PORTB, 11, NO_ADC,       TCC_WO(0, 5), MUX_F, 11 },
  /*  2 LED_3    */ { GUNTHER_PORTB, 23, NO_ADC,       NO_PWM,       0,     7  },
  /*  3 LED_4    */ { GUNTHER_PORTB, 22, NO_ADC,       NO_PWM,       0,     6  },
  /*  4 SWITCH_1 */ { GUNTHER_PORTB,  4, ADC_CH(1, 6), NO_PWM,       0,     4  },
  /*  5 SWITCH_2 */ { GUNTHER_PORTB,  5, ADC_CH(1, 7), NO_PWM,       0,     5  },
  /*  6 SWITCH_3 */ { GUNTHER_PORTB,  6, ADC_CH(1, 8), NO_PWM,       0,     6  },
  /*  7 SWITCH_4 */ { GUNTHER_PORTB,  7, ADC_CH(1, 9), NO_PWM,       0,     7  },
  /*  8 JACK_1   */ { GUNTHER_PORTB,  8, ADC_CH(0, 2), NO_PWM,       0,     8  },
  /*  9 CS_EXT   */ { GUNTHER_PORTA, 11, ADC_CH(0,11), TCC_WO(0, 3), MUX_F, NO_EXTINT },
  /* 10 SERVO_1  */ { GUNTHER_PORTA, 18, NO_ADC,       TCC_WO(1, 2), MUX_F, 2  },
  /* 11 SERVO_2  */ { GUNTHER_PORTA, 19, NO_ADC,       TCC_WO(1, 3), MUX_F, 3  },
  /* 12 SERVO_3  */ { GUNTHER_PORTA, 20, NO_ADC,       TCC_WO(1, 4), MUX_F, 4  },
  /* 13 SERVO_4  */ { GUNTHER_PORTA, 21, NO_ADC,       TCC_WO(1, 5), MUX_F, 5  },
  /* 14 BUZZER   */ { GUNTHER_PORTB, 30, NO_ADC,       TCC_WO(4, 0), MUX_F, 14 },
  /* 15 VBAT     */ { GUNTHER_PORTB,  0, ADC_CH(0,12), NO_PWM,       0,     0  },
  /* 16 TX1      */ { GUNTHER_PORTA, 22, NO_ADC,       NO_PWM,       0,     6  },
  /* 17 RX1      */ { GUNTHER_PORTA, 23, NO_ADC,       NO_PWM,       0,     7  },
  /* 18 SDA      */ { GUNTHER_PORTB,  2, NO_ADC,       NO_PWM,       0,     2  },
  /* 19 SCL      */ { GUNTHER_PORTB,  3, NO_ADC,       NO_PWM,       0,     3  },
  /* 20 BMP_INT  */ { GUNTHER_PORTB, 15, NO_ADC,       NO_PWM,       0,     15 },
  /* 21 MOSI1    */ { GUNTHER_PORTB, 12, NO_ADC,       NO_PWM,       0,     12 },
  /* 22 SCK1     */ { GUNTHER_PORTB, 13, NO_ADC,       NO_PWM,       0,     13 },
  /* 23 MISO1    */ { GUNTHER_PORTB, 14, NO_ADC,       NO_PWM,       0,     14 },
  /* 24 ACC_CS   */ { GUNTHER_PORTA, 16, NO_ADC,       NO_PWM,       0,     0  },
  /* 25 GYR_CS   */ { GUNTHER_PORTA, 17, NO_ADC,       NO_PWM,       0,     1  },
  /* 26 ACC_INT  */ { GUNTHER_PORTB, 16, NO_ADC,       NO_PWM,       0,     0  },
  /* 27 GYR_INT  */ { GUNTHER_PORTB, 17, NO_ADC,       NO_PWM,       0,     1  },
  /* 28 MOSI     */ { GUNTHER_PORTA, 12, NO_ADC,       NO_PWM,       0,     12 },
  /* 29 SCK      */ { GUNTHER_PORTA, 13, NO_ADC,       NO_PWM,       0,     13 },
  /* 30 MISO     */ { GUNTHER_PORTA, 14, NO_ADC,       NO_PWM,       0,     14 },
  /* 31 SD_CS    */ { GUNTHER_PORTA, 15, NO_ADC,       NO_PWM,       0,     15 },
  /* 32 SD_DET   */ { GUNTHER_PORTA, 27, NO_ADC,       NO_PWM,       0,     11 },
  /* 33 USB_DM   */ { GUNTHER_PORTA, 24, NO_ADC,       NO_PWM,       0,     NO_EXTINT },
  /* 34 USB_DP   */ { GUNTHER_PORTA, 25, NO_ADC,       NO_PWM,       0,     NO_EXTINT },
  /* 35 PA02     */ { GUNTHER_PORTA,  2, ADC_CH(0, 0), NO_PWM,       0,     2  },
  /* 36 PA04     */ { GUNTHER_PORTA,  4, ADC_CH(0, 4), NO_PWM,       0,     4  },
  /* 37 PA05     */ { GUNTHER_PORTA,  5, ADC_CH(0, 5), NO_PWM,       0,     5  },
  /* 38 PA06     */ { GUNTHER_PORTA,  6, ADC_CH(0, 6), NO_PWM,       0,     6  },
  /* 39 PA07     */ { GUNTHER_PORTA,  7, ADC_CH(0, 7), NO_PWM,       0,     7  },
  /* 40 LED_RGB  */ { GUNTHER_PORTA,  8, ADC_CH(0, 8), TCC_WO(0, 0), MUX_F, NO_EXTINT },
  /* 41 PA09     */ { GUNTHER_PORTA,  9, ADC_CH(0, 9), TCC_WO(0, 1), MUX_F, 9  },
  /* 42 PA10     */ { GUNTHER_PORTA, 10, ADC_CH(0,10), TCC_WO(0, 2), MUX_F, 10 },
  /* 43 PB09     */ { GUNTHER_PORTB,  9, ADC_CH(0, 3), NO_PWM,       0,     9  },
  /* 44 PB01     */ { GUNTHER_PORTB,  1, ADC_CH(0,13), NO_PWM,       0,     1  },
  /* 45 PB31     */ { GUNTHER_PORTB, 31, NO_ADC,       NO_PWM,       0,     15 },
};

/* Called by the core before setup(): put the board in a safe state */
void initVariant(void)
{
  /* All SPI chip selects high, so no device answers by mistake */
  const uint8_t cs[] = { SD_CS, BMI088_ACCEL_CS, BMI088_GYRO_CS, SPI1_CS_EXT };
  for (unsigned i = 0; i < sizeof(cs); i++) {
    digitalWrite(cs[i], HIGH);
    pinMode(cs[i], OUTPUT);
    digitalWrite(cs[i], HIGH);
  }
  /* Built-in RGB LED off */
  gunther_builtin_rgb(0, 0, 0);
  gunther_builtin_user = 0;

  /* Outputs low at power-up */
  const uint8_t out[] = { LED_1, LED_2, LED_3, LED_4, BUZZER };
  for (unsigned i = 0; i < sizeof(out); i++) {
    pinMode(out[i], OUTPUT);
    digitalWrite(out[i], LOW);
  }
}
