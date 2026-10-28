// Mode classique : la carte devient un esclave I2C (adresse 0x42) pour dialoguer
// avec un autre microcontrôleur branché sur SDA / SCL.
// Attention : en mode esclave, la carte ne peut plus lire son propre baromètre.
//  - le maître écrit 1 octet : 1 = ouvrir la trappe, 0 = fermer
//  - le maître lit 2 octets : tension de la pile en centièmes de volt

#include <Wire.h>
#include <Servo.h>

Servo trappe;
volatile int commande = -1;

void reception(int n) {
  while (Wire.available()) commande = Wire.read();
}

void demande() {
  uint16_t cv = (uint16_t)(readBatteryVoltage() * 100.0f);
  Wire.write(cv >> 8);
  Wire.write(cv & 0xFF);
}

void setup() {
  trappe.attach(SERVO_1);
  Wire.begin(0x42);
  Wire.onReceive(reception);
  Wire.onRequest(demande);
}

void loop() {
  if (commande == 1) trappe.write(90);
  if (commande == 0) trappe.write(0);
}
