// Mode classique : la LED RGB de la carte (LED_BUILTIN, SK6805 sur PA08).
//  - digitalWrite(LED_BUILTIN, HIGH / LOW) : blanc / éteinte, comme une LED normale
//  - analogWrite(LED_BUILTIN, 0..255)      : blanc, luminosité réglable
//  - rgbLedWrite(LED_BUILTIN, r, v, b)      : n'importe quelle couleur
// rgbLedWrite marche aussi avec une LED WS2812 / SK6812 externe sur une autre broche.

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // comme une LED classique
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }

  // couleurs
  rgbLedWrite(LED_BUILTIN, 255, 0, 0);     delay(500);   // rouge
  rgbLedWrite(LED_BUILTIN, 0, 255, 0);     delay(500);   // vert
  rgbLedWrite(LED_BUILTIN, 0, 0, 255);     delay(500);   // bleu
  rgbLedWriteColor(LED_BUILTIN, 0xFF8000); delay(500);   // orange (0xRRGGBB)

  // arc-en-ciel
  for (int h = 0; h < 768; h += 4) {
    int r = h < 256 ? 255 - h : (h < 512 ? 0 : h - 512);
    int g = h < 256 ? h : (h < 512 ? 511 - h : 0);
    int b = h < 256 ? 0 : (h < 512 ? h - 256 : 767 - h);
    rgbLedWrite(LED_BUILTIN, r / 4, g / 4, b / 4);         // / 4 : moins éblouissant
    delay(10);
  }
  rgbLedWrite(LED_BUILTIN, 0, 0, 0);
  delay(1000);
}
