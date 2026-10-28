// Mode classique : faire clignoter une LED, comme sur une Arduino Nano.
// LED_1 .. LED_4 sont les sorties LED du bornier.

void setup() {
  pinMode(LED_1, OUTPUT);
}

void loop() {
  digitalWrite(LED_1, HIGH);
  delay(500);
  digitalWrite(LED_1, LOW);
  delay(500);
}
