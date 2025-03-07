#include <LiquidCrystal.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <FastLED.h>


#define DHTPIN 12
#define DHTTYPE DHT11

// initialisation des capteurs DHT et SGP30
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SGP30 sgp;

//matrice led
#define NUM_LEDS 64
#define DATA_PIN 22
#define BRIGHTNESS 10
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

CRGB leds[NUM_LEDS];


// buzzer
int buzzerPin = 10;

int melody[] = {
  262, 294, 330, 349, 392, 440, 494, 523  // Do, Ré, Mi, Fa, Sol, La, Si, Do
};
int noteDuration = 500;
unsigned long previousMillis = 0;
unsigned long previousMelodyMillis = 0;
const unsigned long interval = 18000;
int noteIndex = 0;
bool isPlaying = false;

//LCD
const int rs = 44, en = 42, d4 = 46, d5 = 41, d6 = 45, d7 = 43;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// pin for presence detector (PIR sensor)
const int PIR_PIN = 38;
int pirState = LOW;

// button
const int BUTTON_PIN = 40;
int buttonState = HIGH;
int lastButtonState = HIGH;
int pressed = 0;
const unsigned long DEBOUNCE_DELAY = 50;
unsigned long lastDebounceTime = 0;

void setup() {

  dht.begin();

  lcd.begin(16, 2);
  pinMode(BUTTON_PIN, INPUT_PULLUP);


  pinMode(PIR_PIN, INPUT);

  if (!sgp.begin()) {
    Serial.println("Erreur : Le capteur SGP30 n'a pas pu être trouvé.");
    while (1)
      ;
  }
  Serial.println("Capteur SGP30 initialisé avec succès.");



  if (!sgp.IAQinit()) {
    Serial.println("Erreur lors de l'initialisation IAQ.");
  }

  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, melody[0], noteDuration);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  turnOffLeds();
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();


  int pirVal = digitalRead(PIR_PIN);
  if (pirVal == HIGH && pirState == LOW) {

    pirState = HIGH;
    lcd.display();
    Serial.println("Présence détectée, écran allumé.");
  } else if (pirVal == LOW && pirState == HIGH) {
    pirState = LOW;
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
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (currentButtonState != buttonState) {
      buttonState = currentButtonState;

      if (buttonState == LOW) {
        pressed++;
        lcd.clear();
        if (pressed > 2) {
          pressed = 0;
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

  } else if (pressed == 1 && pirVal == HIGH) {
    lcd.setCursor(0, 0);
    lcd.print("Humidite: ");
    lcd.setCursor(0, 1);
    lcd.print(humidity);
    lcd.print(" %");
    if (sgp.eCO2 < 1000) {
      drawHumidityDisplay(humidity);
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
    if (sgp.eCO2 > 1000) {
      drawWarningSign();
    } else {
      drawHappySmiley();
    }
  }
  if (sgp.eCO2 > 1000) {
    drawWarningSign();
    if (currentMillis - previousMelodyMillis >= interval && !isPlaying) {
      previousMelodyMillis = currentMillis;
      isPlaying = true;
      noteIndex = 0;
    }

    if (isPlaying) {
      if (currentMillis - previousMillis >= noteDuration) {
        previousMillis = currentMillis;

        noTone(buzzerPin);

        if (noteIndex < 8) {
          tone(buzzerPin, melody[noteIndex], noteDuration);
          noteIndex++;
        } else {
          isPlaying = false;
        }
      }
    }
  }
  if (pirVal == LOW && sgp.eCO2 < 1000) {
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

void drawHappySmiley() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[XY(2, 2)] = CRGB::Yellow;
  leds[XY(5, 2)] = CRGB::Yellow;
  leds[XY(1, 5)] = CRGB::Red;
  leds[XY(2, 6)] = CRGB::Red;
  leds[XY(3, 6)] = CRGB::Red;
  leds[XY(4, 6)] = CRGB::Red;
  leds[XY(5, 6)] = CRGB::Red;
  leds[XY(6, 5)] = CRGB::Red;
  FastLED.show();
}

void drawNeutralSmiley() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[XY(2, 2)] = CRGB::Yellow;
  leds[XY(5, 2)] = CRGB::Yellow;
  leds[XY(2, 6)] = CRGB::Red;
  leds[XY(3, 6)] = CRGB::Red;
  leds[XY(4, 6)] = CRGB::Red;
  leds[XY(5, 6)] = CRGB::Red;

  FastLED.show();
}


void drawSadSmiley() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[XY(2, 2)] = CRGB::Yellow;
  leds[XY(5, 2)] = CRGB::Yellow;
  leds[XY(1, 6)] = CRGB::Red;
  leds[XY(2, 5)] = CRGB::Red;
  leds[XY(3, 5)] = CRGB::Red;
  leds[XY(4, 5)] = CRGB::Red;
  leds[XY(5, 5)] = CRGB::Red;
  leds[XY(6, 6)] = CRGB::Red;

  FastLED.show();
}
void drawWarningSign() {

  fill_solid(leds, NUM_LEDS, CRGB::Black);

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

  FastLED.show();
}

void turnOffLeds() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}
void drawHumidityDisplay(float humidity) {

  if (humidity < 0) humidity = 0;
  if (humidity > 100) humidity = 100;

  int ledsToLightUp = map(humidity, 0, 100, 0, NUM_LEDS);

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (int i = 0; i < ledsToLightUp; i++) {
    leds[i] = CRGB::Blue;
  }

  FastLED.show();
}
