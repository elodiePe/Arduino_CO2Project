// include the library code:
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <FastLED.h>

// définir le type de capteur DHT et la broche de données
#define DHTPIN 12
#define DHTTYPE DHT11

// initialisation des capteurs DHT et SGP30
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SGP30 sgp;

// définir Matrice LED 8x8
#define NUM_LEDS 64      // Nombre de LEDs (8x8 = 64 LEDs)
#define DATA_PIN 22      // Pin connecté au DIN de la matrice WS2812B
#define BRIGHTNESS 10    // Luminosité (0-255)
#define MATRIX_WIDTH 8   // Largeur de la matrice (8 colonnes)
#define MATRIX_HEIGHT 8  // Hauteur de la matrice (8 lignes)

CRGB leds[NUM_LEDS];  // Tableau contenant les données des LEDs


// Pin du buzzer
int buzzerPin = 10;

// Notes musicales et durées des notes (en millisecondes)
int melody[] = {
  262, 294, 330, 349, 392, 440, 494, 523  // Do, Ré, Mi, Fa, Sol, La, Si, Do
};

int noteDuration = 500;

// Variables pour la gestion du temps pour la musique
unsigned long previousMillis = 0;        // Temps de la dernière note jouée
unsigned long previousMelodyMillis = 0;  // Temps de la dernière répétition de la mélodie
const unsigned long interval = 18000;    // Intervalle de 3 minutes (en millisecondes)
int noteIndex = 0;                       // Indice de la note en cours
bool isPlaying = false;                  // Pour savoir si la mélodie est en cours de lecture

// initialize the library by associating any needed LCD interface pin
// with the Arduino pin number it is connected to
const int rs = 44, en = 42, d4 = 46, d5 = 41, d6 = 45, d7 = 43;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// pin for presence detector (PIR sensor)
const int PIR_PIN = 38;
int pirState = LOW;  // LOW means no motion detected

// button
const int BUTTON_PIN = 40;
int buttonState = HIGH;
int lastButtonState = HIGH;
int pressed = 0;  // Indicateur d'état d'affichage (0 pour Temp, 1 pour Humidité, 2 pour CO2/TVOC)
const unsigned long DEBOUNCE_DELAY = 50;
unsigned long lastDebounceTime = 0;

void setup() {
  // Démarrer le capteur DHT
  dht.begin();
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // PIR sensor (presence detector) setup
  pinMode(PIR_PIN, INPUT);
  // sgp.begin();
  // Démarrage du capteur SGP30
  if (!sgp.begin()) {
    Serial.println("Erreur : Le capteur SGP30 n'a pas pu être trouvé.");
    while (1)
      ;
  }
  Serial.println("Capteur SGP30 initialisé avec succès.");

  // Démarrer la mesure d'air TVOC et eCO2
  // sgp.IAQinit();
  if (!sgp.IAQinit()) {
    Serial.println("Erreur lors de l'initialisation IAQ.");
  }

  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, melody[0], noteDuration);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);  // Ajuste la luminosité
  turnOffLeds();
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  // Gestion du détecteur de présence (PIR)
  int pirVal = digitalRead(PIR_PIN);
  if (pirVal == HIGH && pirState == LOW) {

    pirState = HIGH;  // Présence détectée
    lcd.display();
    Serial.println("Présence détectée, écran allumé.");
  } else if (pirVal == LOW && pirState == HIGH) {
    pirState = LOW;  // Plus de présence
    lcd.noDisplay();
    turnOffLeds();
    Serial.println("Pas de présence, écran éteint.");
  }

  if (!sgp.IAQmeasure()) {
    Serial.println("Erreur lors de la mesure de la qualité de l'air.");
    return;
  };

  // Gestion du bouton
  int currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();  // Reset du temps de debounce
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (currentButtonState != buttonState) {
      buttonState = currentButtonState;

      if (buttonState == LOW) {
        pressed++;  // Passer à l'état suivant (0 -> 1 -> 2 -> 0)
        lcd.clear();
        if (pressed > 2) {
          pressed = 0;  // Remettre à 0 après le troisième état
        }
        Serial.println("🔘 Appuyé !");
      }
    }
  }

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (pressed == 0 && pirVal == HIGH) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.setCursor(0, 1);
    lcd.print(temperature);
    lcd.print(" C");
    if (temperature < 10 && sgp.eCO2 < 1000) {
      drawSadSmiley();
    };
    if (temperature < 20 && temperature > 10 && sgp.eCO2 < 1000) {
      drawNeutralSmiley();
    }
    if (temperature > 20 && sgp.eCO2 < 1000) {
      drawHappySmiley();
    }
    if (temperature == 20) {
      drawHappySmiley;
    }

  } else if (pressed == 1 && pirVal == HIGH ) {
    lcd.setCursor(0, 0);
    lcd.print("Humidite: ");
    lcd.setCursor(0, 1);
    lcd.print(humidity);
    lcd.print(" %");
    if (sgp.eCO2 < 1000) {
  drawHumidityDisplay(humidity); // Afficher l'humidité sur le panneau LED

    }
  } else if (pressed == 2 && pirVal == HIGH) {
    lcd.setCursor(0, 0);
    lcd.print("CO2: ");
    lcd.print(sgp.eCO2);
    lcd.print(" ppm");
    lcd.setCursor(0, 1);
    lcd.print("TVOC: ");
    lcd.print(sgp.TVOC);
    lcd.print(" ppb");
    if (sgp.eCO2 > 1000){
    drawWarningSign();
    } else {
      drawHappySmiley();
    }


  } 
  if (sgp.eCO2 > 1000) {
    drawWarningSign();
    // Si le temps écoulé depuis la dernière note est supérieur à la durée de la note actuelle
    if (currentMillis - previousMelodyMillis >= interval && !isPlaying) {
      // Si l'intervalle est atteint, commence à jouer la mélodie
      previousMelodyMillis = currentMillis;  // Réinitialise le compteur pour 5 minutes
      isPlaying = true;                      // Indique que la mélodie est en cours de lecture
      noteIndex = 0;                         // Réinitialise l'indice de la mélodie
    }

    // Si la mélodie est en train de jouer, on vérifie si la prochaine note doit être jouée
    if (isPlaying) {
      // Si le temps écoulé depuis la dernière note est supérieur à la durée de la note actuelle
      if (currentMillis - previousMillis >= noteDuration) {
        // Met à jour le temps de référence pour la prochaine note
        previousMillis = currentMillis;

        // Arrête de jouer la note précédente
        noTone(buzzerPin);

        // Joue la prochaine note si on n'est pas à la fin de la mélodie
        if (noteIndex < 8) {
          tone(buzzerPin, melody[noteIndex], noteDuration);
          noteIndex++;  // Passe à la note suivante
        } else {
          // Si la mélodie est terminée, on arrête de jouer
          isPlaying = false;
        }
      }
    }
  }
  if (pirVal == LOW && sgp.eCO2 <1000) {
    turnOffLeds();
  }
  lastButtonState = currentButtonState;
}

int XY(int x, int y) {
  if (y % 2 == 0) {
    return (y * MATRIX_WIDTH) + x;
  } else {
    return (y * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - x);
  }
}

// Fonction pour dessiner un smiley heureux
void drawHappySmiley() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[XY(2, 2)] = CRGB::Yellow;  // Oeil gauche
  leds[XY(5, 2)] = CRGB::Yellow;  // Oeil droit
  leds[XY(1, 5)] = CRGB::Red;
  leds[XY(2, 6)] = CRGB::Red;
  leds[XY(3, 6)] = CRGB::Red;
  leds[XY(4, 6)] = CRGB::Red;
  leds[XY(5, 6)] = CRGB::Red;
  leds[XY(6, 5)] = CRGB::Red;
  FastLED.show();
}

// Fonction pour dessiner un smiley triste
void drawNeutralSmiley() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[XY(2, 2)] = CRGB::Yellow;  // Oeil gauche
  leds[XY(5, 2)] = CRGB::Yellow;  // Oeil droit
  leds[XY(2, 6)] = CRGB::Red;     // Coin droit de la bouche
  leds[XY(3, 6)] = CRGB::Red;     // Bouche triste
  leds[XY(4, 6)] = CRGB::Red;
  leds[XY(5, 6)] = CRGB::Red;  // Coin droit de la bouche

  FastLED.show();
}

// Fonction pour dessiner un smiley neutre
void drawSadSmiley() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[XY(2, 2)] = CRGB::Yellow;  // Oeil gauche
  leds[XY(5, 2)] = CRGB::Yellow;  // Oeil droit
  leds[XY(1, 6)] = CRGB::Red;     // Coin gauche de la bouche
  leds[XY(2, 5)] = CRGB::Red;     // Partie inférieure gauche de la bouche
  leds[XY(3, 5)] = CRGB::Red;     // Partie inférieure droite de la bouche
  leds[XY(4, 5)] = CRGB::Red;     // Coin droit de la bouche
  leds[XY(5, 5)] = CRGB::Red;     // Coin droit de la bouche
  leds[XY(6, 6)] = CRGB::Red;     // Coin droit de la bouche

  FastLED.show();
}
void drawWarningSign() {
  // Efface toutes les LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Dessine le contour du triangle (couleur jaune)
  leds[XY(3, 1)] = CRGB::Red;
  leds[XY(3, 2)] = CRGB::Red;
  leds[XY(3, 3)] = CRGB::Red;
  leds[XY(3, 4)] = CRGB::Red;
  leds[XY(4, 1)] = CRGB::Red;
  leds[XY(4, 2)] = CRGB::Red;
  leds[XY(4, 3)] = CRGB::Red;
  leds[XY(4, 4)] = CRGB::Red;

  leds[XY(3, 6)] = CRGB::Red;
  leds[XY(4, 6)] = CRGB::Red;

  // Afficher les LEDs
  FastLED.show();
}

void turnOffLeds() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}
void drawHumidityDisplay(float humidity) {
  // Limiter l'humidité à 0% - 100% pour éviter des valeurs invalides
  if (humidity < 0) humidity = 0;
  if (humidity > 100) humidity = 100;

  // Calculer le nombre de LEDs à allumer en fonction du taux d'humidité
  int ledsToLightUp = map(humidity, 0, 100, 0, NUM_LEDS); // Calcul du nombre de LEDs allumées

  fill_solid(leds, NUM_LEDS, CRGB::Black); // Éteindre toutes les LEDs au départ

  // Allumer les LEDs proportionnellement à l'humidité
  for (int i = 0; i < ledsToLightUp; i++) {
    leds[i] = CRGB::Blue;  // Allumer la LED en bleu
  }

  FastLED.show();  // Afficher l'état actuel des LEDs
}

