# Temperature et humidité
- [Lien vers la documentation](https://projecthub.arduino.cc/rudraksh2008/temperature-and-humidity-sensor-with-arduino-1d52a6)
## exemple de code
```c++
// Inclure les bibliothèques DHT et Adafruit Unified Sensor

#include <DHT.h>
#include <Adafruit_Sensor.h>

// Définir le type de capteur DHT et la broche de données

#define DHTPIN 3    
// Pin à laquelle le DHT est connecté

#define DHTTYPE DHT11   // DHT 11 (ou utilisez DHT22 pour le capteur DHT22)

// Initialisation du capteur DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Démarrer la communication série
  Serial.begin(9600);
  Serial.println("DHT11 Sensor - Temperature and Humidity");
  
  // Démarrer le capteur DHT
dht.begin();
}
void loop() {

  // Attendre 2 secondes entre les mesures
  delay(2000);
  
  // Lire l'humidité
  float humidity = dht.readHumidity();

  // Lire la température en Celsius
  float temperature = dht.readTemperature();
  
  // Vérifier si les lectures ont échoué et sortir si c'est le cas
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Erreur de lecture du capteur DHT!");
    return;
  }
  
  // Afficher les résultats sur le moniteur série
  Serial.print("Humidité: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Température: ");
  Serial.print(temperature);
  Serial.println(" °C");
}
```