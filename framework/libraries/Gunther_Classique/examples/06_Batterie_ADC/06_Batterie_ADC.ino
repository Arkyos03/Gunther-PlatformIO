// Mode classique : mesure de la tension de la pile avec analogRead().
// VBAT_SENSE (= A0) reçoit la tension de la pile divisée par 4 (pont 30k / 10k).

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);         // 0..4095 (10 bits par défaut, comme une Nano)
  analogReference(AR_DEFAULT);      // référence 3,3 V (voir aussi AR_INTERNAL2V5...)
}

void loop() {
  int brut = analogRead(VBAT_SENSE);
  float vBroche = brut * 3.3 / 4095.0;
  float vPile = vBroche * VBAT_DIVIDER_RATIO;

  Serial.print("ADC = ");
  Serial.print(brut);
  Serial.print("   pile = ");
  Serial.print(vPile, 2);
  Serial.print(" V   (readBatteryVoltage() = ");
  Serial.print(readBatteryVoltage(), 2);   // même calcul, moyenné sur 8 mesures
  Serial.println(" V)");
  delay(500);
}
