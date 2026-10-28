// LED RGB de la carte : animations automatiques et couleurs personnalisées.
//
// Après Gunther.begin(), la LED RGB montre toute seule ce que fait la carte :
//   bleu qui respire       démarrage
//   3 flashs verts         auto-test OK (avec les 3 bips)
//   flashs rouges          erreur : 1 = baromètre, 2 = centrale, 3 = carte SD, 4 = pile
//   vert qui respire       prête, enregistrement en cours
//   cyan qui respire       prête, sans enregistrement
//   orange qui clignote    pile faible
//   flash blanc            décollage, apogée
//   bleu rapide            montée
//   violet rapide          descente
//   double flash blanc     posée (balise pour retrouver la fusée)
//
// Envoyer dans le moniteur série :
//   r / v / b  -> la LED en rouge, vert, bleu (le programme prend la main)
//   a          -> retour aux animations automatiques
//   f          -> un flash jaune (événement personnalisé)
//   l          -> simulation : décollage, puis apogée 3 s plus tard

#include <Gunther.h>

void setup() {
  Serial.begin(115200);
  Gunther.begin();
  Gunther.statusBrightness(60);      // 0..255
  // Gunther.setStatusLed(SPI1_CS_EXT);  // animations sur une LED WS2812 externe
  // Gunther.statusLed(false);           // pas d'animation du tout
  Gunther.selfTest(false, false);
}

void loop() {
  Gunther.update();
  switch (Serial.read()) {
    case 'r': Gunther.rgb(255, 0, 0); break;
    case 'v': Gunther.rgb(0, 255, 0); break;
    case 'b': Gunther.rgb(0x0000FF); break;
    case 'a': Gunther.statusLed(true); break;
    case 'f': Gunther.statusFlash(0xFFC000, 300); break;
    case 'l':
      Gunther.setApogeeTimeout(3000);
      Gunther.setApogeeMinTime(10000);    // l'apogée vient de la sécurité à 3 s
      Gunther.resetFlight();
      Serial.println("arracher le jack...");
      break;
  }
}
