// Fait clignoter la LED branchée sur le bornier LED 1
// (LED + résistance entre la borne LED_1 et GND)

void setup() {
  pinMode(LED_1, OUTPUT);
}

void loop() {
  digitalWrite(LED_1, HIGH);   // allume
  delay(500);                  // attend 0,5 s
  digitalWrite(LED_1, LOW);    // éteint
  delay(500);
}
