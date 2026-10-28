// Mode classique : buzzer avec tone() et noTone().

void setup() {
  tone(BUZZER, 2000, 100);          // 2 kHz pendant 100 ms
  delay(300);
  tone(BUZZER, 4000, 100);
  delay(300);
}

void loop() {
  // sirène
  for (int f = 2000; f <= 4500; f += 50) {
    tone(BUZZER, f);
    delay(5);
  }
  noTone(BUZZER);
  delay(2000);
}
