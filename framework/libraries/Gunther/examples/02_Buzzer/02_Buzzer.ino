// Le buzzer de la carte joue un bip toutes les secondes

void setup() {
}

void loop() {
  tone(BUZZER, 4000, 150);   // 4000 Hz pendant 150 ms
  delay(1000);
}
