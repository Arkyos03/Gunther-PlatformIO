// Mode classique : variation de luminosité avec analogWrite (PWM).
// PWM disponible sur LED_1, LED_2 et BUZZER (732 Hz), et sur SERVO_1..4 (50 Hz).

void setup() {
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
}

void loop() {
  for (int i = 0; i <= 255; i++) {
    analogWrite(LED_1, i);
    analogWrite(LED_2, 255 - i);
    delay(4);
  }
  for (int i = 255; i >= 0; i--) {
    analogWrite(LED_1, i);
    analogWrite(LED_2, 255 - i);
    delay(4);
  }
}
