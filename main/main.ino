// include the library code:
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>

// définir le type de capteur DHT et la broche de données
#define DHTPIN 12
#define DHTTYPE DHT11

// initialisation des capteurs DHT et SGP30
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SGP30 sgp;

// Pin du buzzer
int buzzerPin = 10;

// Notes musicales et durées des notes (en millisecondes)
int melody[] = {
  262, 294, 330, 349, 392, 440, 494, 523  // Do, Ré, Mi, Fa, Sol, La, Si, Do
};

int noteDuration = 500;

// Variables pour la gestion du temps
unsigned long previousMillis = 0;              // Temps de la dernière note jouée
unsigned long previousMelodyMillis = 0;        // Temps de la dernière répétition de la mélodie
const unsigned long interval = 180000;  // Intervalle de 3 minutes (en millisecondes)
int noteIndex = 0;                             // Indice de la note en cours
bool isPlaying = false;                        // Pour savoir si la mélodie est en cours de lecture

// initialize the library by associating any needed LCD interface pin
// with the Arduino pin number it is connected to
const int rs = 44, en = 42, d4 = 46, d5 = 41, d6 = 45, d7 = 43;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
byte customChar[8] = {
  B00100,
  B00100,
  B01110,
  B10101,
  B11111,
  B00100,
  B01010,
  B10001
};
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

const int LEDARRAY_D = 2;
const int LEDARRAY_C = 3;
const int LEDARRAY_B = 4;
const int LEDARRAY_A = 5;
const int LEDARRAY_G = 6;
const int LEDARRAY_DI = 7;
const int LEDARRAY_CLK = 8;
const int LEDARRAY_LAT = 9;

unsigned char displayBuffer[8];
// Déclaration du tableau avec le bon type
const unsigned char happySmiley[2][32] = {
  0xF8,
  0xE0,
  0xC0,
  0x86,
  0x86,
  0x06,
  0x00,
  0x20,
  0x30,
  0x18,
  0x8C,
  0x87,
  0xC0,
  0xE0,
  0xF8,
  0xFF,
  0x3F,
  0x0F,
  0x07,
  0xC3,
  0xC3,
  0xC1,
  0x01,
  0x09,
  0x19,
  0x31,
  0x63,
  0xC3,
  0x07,
  0x0F,
  0x3F,
  0xFF,
};

const unsigned char sunSmiley[2][32] = {
  // SMILEY SUN
  0xFF,
  0xFF,
  0x02,
  0x80,
  0x82,
  0x87,
  0xFF,
  0xFF,
  0x3F,
  0xDF,
  0xCF,
  0xEF,
  0xF0,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x07,
  0x0F,
  0x0F,
  0x0F,
  0xFF,
  0xFF,
  0xE7,
  0xCF,
  0x9F,
  0xBF,
  0x7F,
  0xFF,
  0xFF,
  0xFF,
};

const unsigned char coldSmiley[2][32] = {
  // COLD SMILEY
  0xFF,
  0xFF,
  0xE7,
  0xDB,
  0xFF,
  0xE7,
  0xE7,
  0xFF,
  0xFF,
  0xC0,
  0xD6,
  0xDA,
  0xC0,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xCF,
  0xB7,
  0xFF,
  0xCF,
  0xCF,
  0xFF,
  0xFF,
  0x07,
  0xD7,
  0xB7,
  0x07,
  0xFF,
  0xFF,
  0xFF,
};
const unsigned char warning[2][32] = {
  // WARNING SIGN
  0xFF,
  0xFE,
  0xFD,
  0xFB,
  0xFA,
  0xF6,
  0xF6,
  0xE6,
  0xEE,
  0xEF,
  0xCE,
  0xDE,
  0x9F,
  0xBF,
  0x80,
  0xFF,
  0x7F,
  0x3F,
  0xDF,
  0xEF,
  0x2F,
  0x37,
  0x37,
  0x33,
  0x3B,
  0xFB,
  0x39,
  0x3D,
  0x03,
  0x01,
  0x00,
  0xFF,
};
void setup() {
  // Démarrer le capteur DHT
  dht.begin();
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, customChar);
  // lcd.noDisplay();  // Ecran éteint au démarrage
  // Message initial sur la ligne 2 (ligne 1 dans le code, car l'index commence à 0)
  lcd.setCursor(0, 1);  // Place le curseur en bas
  lcd.print("Animation demo");
  // button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // PIR sensor (presence detector) setup
  pinMode(PIR_PIN, INPUT);

  // Démarrage du capteur SGP30
  if (!sgp.begin()) {
    Serial.println("Erreur : Le capteur SGP30 n'a pas pu être trouvé.");
    while (1)
      ;
  }
  Serial.println("Capteur SGP30 initialisé avec succès.");

  // Démarrer la mesure d'air TVOC et eCO2
  if (!sgp.IAQinit()) {
    Serial.println("Erreur lors de l'initialisation IAQ.");
  }
  pinMode(LEDARRAY_D, OUTPUT);
  pinMode(LEDARRAY_C, OUTPUT);
  pinMode(LEDARRAY_B, OUTPUT);
  pinMode(LEDARRAY_A, OUTPUT);
  pinMode(LEDARRAY_G, OUTPUT);
  pinMode(LEDARRAY_DI, OUTPUT);
  pinMode(LEDARRAY_CLK, OUTPUT);
  pinMode(LEDARRAY_LAT, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, melody[0], noteDuration);
  display(happySmiley);  // Affiche directement le visage souriant

  //  for (int i = 0; i < 16; i++) {
  // lcd.clear();  // Efface l'écran pour créer l'illusion de mouvement
  // lcd.setCursor(i, 0);  // Déplace le curseur à la position i (ligne 0)
  // lcd.write(byte(0));  // Affiche le caractère personnalisé (petite voiture)
  // lcd.setCursor(0, 1);  // Affiche le texte fixe sur la 2ème ligne
  // lcd.print("Animation demo");
  // delay(200);  // Pause entre chaque déplacement
}

void loop() {
  // Récupère l'heure actuelle en millisecondes
  unsigned long currentMillis = millis();


  // Vérification du détecteur de présence (PIR)
  int pirVal = digitalRead(PIR_PIN);

  // Si le détecteur de présence est activé (quelqu'un est détecté)
  if (pirVal == HIGH && pirState == LOW) {
    pirState = HIGH;  // Met à jour l'état du PIR
    lcd.display();    // Allume l'écran LCD
    Serial.println("Présence détectée, écran allumé.");




  } else if (pirVal == LOW && pirState == HIGH) {
    pirState = LOW;   // Met à jour l'état du PIR
    lcd.noDisplay();  // Éteint l'écran LCD

    Serial.println("Pas de présence, écran éteint.");
  };

  // Vérification si le capteur SGP30 peut mesurer
  if (!sgp.IAQmeasure()) {
    Serial.println("Erreur lors de la mesure de la qualité de l'air.");
    return;
  };

  // Gestion du bouton
  int currentButtonState = digitalRead(BUTTON_PIN);

  // On vérifie si le bouton a changé d’état
  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();  // Reset du temps de debounce
  }
  // Lecture des valeurs du capteur
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  // On vérifie si l'état du bouton est stable après le délai de debounce
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // Si l'état a changé après le délai, on le prend en compte
    if (currentButtonState != buttonState) {
      buttonState = currentButtonState;

      if (buttonState == LOW) {
        pressed++;  // Passer à l'état suivant (0 -> 1 -> 2 -> 0)
        if (pressed > 3) {
          pressed = 0;  // Remettre à 0 après le troisième état
        }

        // changeDisplay(happySmiley);
        Serial.println("🔘 Appuyé !");
      }
    }
  }

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Erreur de lecture du capteur DHT");
    return;
  }

  // Affichage en fonction de l'état de `pressed`
  if (pressed == 0) {
    // Affichage de la température
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.setCursor(0, 1);
    lcd.print(temperature);
    lcd.print(" C");

    if (sgp.eCO2 < 1000) {
      if (temperature > 20) {
        display(sunSmiley);
      } else {
        display(coldSmiley);
      }
    }
  } else if (pressed == 1) {

    // Affichage de l'humidité
    lcd.setCursor(0, 0);
    lcd.print("Humidite: ");
    lcd.setCursor(0, 1);
    lcd.print(humidity);
    lcd.print(" %");
  } else if (pressed == 2) {
    // Affichage du CO2 et des COV (TVOC)
    lcd.setCursor(0, 0);
    lcd.print("CO2: ");
    lcd.print(sgp.eCO2);
    lcd.print(" ppm");
    lcd.setCursor(0, 1);
    lcd.print("TVOC: ");
    lcd.print(sgp.TVOC);
    lcd.print(" ppb");
  } else if (pressed == 3) {
    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.print("heure et date");
  }
  if (sgp.eCO2 > 1000) {
    display(warning);
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

  // Mise à jour de l'état précédent du bouton
  lastButtonState = currentButtonState;
}

void scanLine(unsigned char m) {
  switch (m) {
    case 0:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 1:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    case 2:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 3:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    case 4:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 5:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    case 6:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 7:
      digitalWrite(LEDARRAY_D, LOW);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    case 8:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 9:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    case 10:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 11:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, LOW);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    case 12:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 13:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, LOW);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    case 14:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, LOW);
      break;
    case 15:
      digitalWrite(LEDARRAY_D, HIGH);
      digitalWrite(LEDARRAY_C, HIGH);
      digitalWrite(LEDARRAY_B, HIGH);
      digitalWrite(LEDARRAY_A, HIGH);
      break;
    default:
      break;
  }
}

void send(unsigned char dat) {
  unsigned char i;
  digitalWrite(LEDARRAY_CLK, LOW);
  delayMicroseconds(1);
  digitalWrite(LEDARRAY_LAT, LOW);
  delayMicroseconds(1);

  for (i = 0; i < 8; i++) {
    if (dat & 0x01) {
      digitalWrite(LEDARRAY_DI, HIGH);
    } else {
      digitalWrite(LEDARRAY_DI, LOW);
    }
    digitalWrite(LEDARRAY_CLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(LEDARRAY_CLK, LOW);
    delayMicroseconds(1);
    dat >>= 1;
  }
}

void display(const unsigned char dat[][32]) {
  unsigned char i;

  for (i = 0; i < 16; i++) {
    digitalWrite(LEDARRAY_G, HIGH);

    displayBuffer[0] = dat[0][i];
    displayBuffer[1] = dat[0][i + 16];
    displayBuffer[2] = dat[1][i];
    displayBuffer[3] = dat[1][i + 16];

    displayBuffer[4] = dat[2][i];
    displayBuffer[5] = dat[2][i + 16];
    displayBuffer[6] = dat[3][i];
    displayBuffer[7] = dat[3][i + 16];

    send(displayBuffer[7]);
    send(displayBuffer[6]);
    send(displayBuffer[5]);
    send(displayBuffer[4]);

    send(displayBuffer[3]);
    send(displayBuffer[2]);
    send(displayBuffer[1]);
    send(displayBuffer[0]);

    digitalWrite(LEDARRAY_LAT, HIGH);
    delayMicroseconds(1);

    digitalWrite(LEDARRAY_LAT, LOW);
    delayMicroseconds(1);

    scanLine(i);

    digitalWrite(LEDARRAY_G, LOW);

    delayMicroseconds(300);
  }
}