// Mode classique : UART d'extension Serial1 (TX1 = PA22, RX1 = PA23).
// Tout ce qui arrive sur l'USB est renvoyé sur Serial1 et inversement :
// pratique pour configurer un module radio ou un GPS.

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1);
}

void loop() {
  while (Serial.available())  Serial1.write(Serial.read());
  while (Serial1.available()) Serial.write(Serial1.read());
}
