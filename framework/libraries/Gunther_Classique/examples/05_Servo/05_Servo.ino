// Mode classique : servomoteur avec la bibliothèque Servo standard.
// Fonctionne sur SERVO_1 .. SERVO_4.

#include <Servo.h>

Servo trappe;

void setup() {
  trappe.attach(SERVO_1);           // impulsion 544..2400 µs par défaut
  // trappe.attach(SERVO_1, 1000, 2000);  // ou bornes personnalisées
}

void loop() {
  trappe.write(0);                  // angle en degrés
  delay(1000);
  trappe.write(90);
  delay(1000);
  trappe.writeMicroseconds(1500);   // ou directement en microsecondes
  delay(1000);
}
