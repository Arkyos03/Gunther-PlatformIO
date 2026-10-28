// Mode classique : lire les switchs et le jack, interruption sur le jack.
//  - switch entre la broche et GND : INPUT_PULLUP, fermé = LOW
//  - jack : résistance de 10k vers GND sur la carte, branché = HIGH, arraché = LOW

volatile bool jackArrache = false;
volatile uint32_t instantDecollage = 0;

void jackISR() {                    // appelée dès que le jack est arraché
  if (!jackArrache) {
    jackArrache = true;
    instantDecollage = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SWITCH_1, INPUT_PULLUP);
  pinMode(SWITCH_2, INPUT_PULLUP);
  pinMode(SWITCH_3, INPUT_PULLUP);
  pinMode(SWITCH_4, INPUT_PULLUP);
  pinMode(JACK_1, INPUT_PULLDOWN);
  pinMode(LED_1, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(JACK_1), jackISR, FALLING);
}

void loop() {
  Serial.print("switchs : ");
  Serial.print(digitalRead(SWITCH_1) == LOW ? "1 " : "- ");
  Serial.print(digitalRead(SWITCH_2) == LOW ? "2 " : "- ");
  Serial.print(digitalRead(SWITCH_3) == LOW ? "3 " : "- ");
  Serial.print(digitalRead(SWITCH_4) == LOW ? "4 " : "- ");
  Serial.print("  jack : ");
  Serial.println(digitalRead(JACK_1) == HIGH ? "branche" : "arrache");

  if (jackArrache) {
    digitalWrite(LED_1, HIGH);
    Serial.print("decollage a t = ");
    Serial.print(instantDecollage);
    Serial.println(" ms");
  }
  delay(200);
}
