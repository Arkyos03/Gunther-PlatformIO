// Affiche les valeurs des capteurs dans le moniteur série (ou le traceur série)

#include <Gunther.h>

void setup() {
  Serial.begin(115200);
  Gunther.begin();                  // l'altitude 0 est prise ici
}

void loop() {
  float ax, ay, az, gx, gy, gz;
  Gunther.accel(ax, ay, az);        // en m/s²
  Gunther.gyro(gx, gy, gz);         // en °/s

  Serial.printf("alt %.2f m  temp %.1f C  acc %.2f %.2f %.2f  gyro %.1f %.1f %.1f  g %.2f\n",
                Gunther.altitude(), Gunther.temperature(), ax, ay, az, gx, gy, gz, Gunther.gForce());
  delay(100);
}
