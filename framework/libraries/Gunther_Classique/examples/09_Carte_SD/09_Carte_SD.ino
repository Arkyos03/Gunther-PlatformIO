// Mode classique : écrire un fichier CSV sur la carte micro SD avec SdFat.
// Bus SPI, CS = SD_CS.
// Sur la Rev A, la broche SD_DETECT ne fonctionne pas (elle reste à HIGH) :
// on sait qu'une carte est présente quand sd.begin() réussit.

#include <SPI.h>
#include <SdFat.h>

SdFat sd;
FsFile fichier;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}
  if (!sd.begin(SdSpiConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(12), &SPI))) {
    Serial.println("pas de carte SD, ou carte illisible");
    while (true) {}
  }
  if (!fichier.open("mesures.csv", O_WRONLY | O_CREAT | O_APPEND)) {
    Serial.println("impossible d'ouvrir le fichier");
    while (true) {}
  }
  fichier.println("temps_ms;pile_V");
  Serial.println("enregistrement...");
}

void loop() {
  fichier.print(millis());
  fichier.print(';');
  fichier.println(readBatteryVoltage(), 3);

  static uint32_t dernier = 0;
  if (millis() - dernier >= 500) {  // écriture réelle sur la carte toutes les 0,5 s
    dernier = millis();
    fichier.sync();
  }
  if (millis() > 20000) {           // arrêt au bout de 20 s
    fichier.close();
    Serial.println("termine");
    while (true) {}
  }
  delay(50);
}
