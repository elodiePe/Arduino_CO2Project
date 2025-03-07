# Détecteur de CO2
- [Lien vers la documentation du SGP30](https://cdn-learn.adafruit.com/downloads/pdf/adafruit-sgp30-gas-tvoc-eco2-mox-sensor.pdf)

## Exemple de code
```c++
#include <Wire.h>
#include "Adafruit_SGP30.h"

// Initialisation du capteur SGP30
Adafruit_SGP30 sgp;

void setup() {
  Serial.begin(9600);
  Serial.println("Initialisation du capteur SGP30...");

  if (!sgp.begin()) {
    Serial.println("Erreur : Le capteur SGP30 n'a pas pu être trouvé.");
    while (1);
  }
  Serial.println("Capteur SGP30 initialisé avec succès.");

  // Démarrer la mesure d'air TVOC et eCO2
  if (!sgp.IAQinit()) {
    Serial.println("Erreur lors de l'initialisation IAQ.");
  }
}

void loop() {
  if (!sgp.IAQmeasure()) {
    Serial.println("Erreur lors de la mesure de la qualité de l'air.");
    return;
  }

  // Affichage de la concentration de CO2 équivalent (eCO2) en ppm
  Serial.print("CO2 équivalent (eCO2) : ");
  Serial.print(sgp.eCO2);
  Serial.println(" ppm");

  // Affichage de la concentration de COV totaux (TVOC) en ppb
  Serial.print("COV totaux (TVOC) : ");
  Serial.print(sgp.TVOC);
  Serial.println(" ppb");

  delay(1000); // Attendre 1 seconde avant la prochaine lecture
}

```