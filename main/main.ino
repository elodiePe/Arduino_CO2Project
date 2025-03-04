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
// Définition des broches pour la matrice LED
const int LEDARRAY_D = 2;
const int LEDARRAY_C = 3;
const int LEDARRAY_B = 4;
const int LEDARRAY_A = 5;
const int LEDARRAY_G = 6;
const int LEDARRAY_DI = 7;
const int LEDARRAY_CLK = 8;
const int LEDARRAY_LAT = 9;

// Définition des constantes pour l'affichage
const int DISPLAY_NUM_WORD = 2;
const int NUM_OF_WORD = 2;
unsigned char displayBuffer[8];
unsigned char displaySwapBuffer[DISPLAY_NUM_WORD][32] = { 0 };
bool shiftBit = 0;
bool flagShift = 0;
unsigned char timerCount = 0;
unsigned char temp = 0x80;
unsigned char shiftCount = 0;
unsigned char displayWordCount = 0;
bool smileyDisplayed = false;
bool animationActive = false;
bool animationComplete = false;

// Définition du visage souriant
const char happySmiley[2][32] = {
  // hAPPY sMILEY
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

// Pointeur vers le message actuel
const char (*MESSAGE)[32];

// Fonction pour changer l'affichage
void changeDisplay(const char (*newMessage)[32]) {
  MESSAGE = newMessage;
  cleardisplay();
}

// Fonction pour effacer l'affichage
void cleardisplay() {
  for (int j = 0; j < DISPLAY_NUM_WORD; j++) {
    for (int i = 0; i < 32; i++) {
      displaySwapBuffer[j][i] = 0xFF;
    }
  }
}

// Fonction pour calculer le défilement
void calcShift() {
  unsigned char i;
  for (i = 0; i < 16; i++) {
    if ((displaySwapBuffer[0][16 + i] & 0x80) == 0) {
      displaySwapBuffer[0][i] = (displaySwapBuffer[0][i] << 1) & 0xfe;
    } else {
      displaySwapBuffer[0][i] = (displaySwapBuffer[0][i] << 1) | 0x01;
    }
    if ((displaySwapBuffer[1][i] & 0x80) == 0) {
      displaySwapBuffer[0][16 + i] = (displaySwapBuffer[0][16 + i] << 1) & 0xfe;
    } else {
      displaySwapBuffer[0][16 + i] = (displaySwapBuffer[0][16 + i] << 1) | 0x01;
    }
    if ((displaySwapBuffer[1][16 + i] & 0x80) == 0) {
      displaySwapBuffer[1][i] = (displaySwapBuffer[1][i] << 1) & 0xfe;
    } else {
      displaySwapBuffer[1][i] = (displaySwapBuffer[1][i] << 1) | 0x01;
    }
    if (shiftCount % 16 < 8 && displayWordCount < NUM_OF_WORD) {
      shiftBit = MESSAGE[displayWordCount][i] & temp;
    } else if (shiftCount % 16 < 16 && displayWordCount < NUM_OF_WORD) {
      shiftBit = MESSAGE[displayWordCount][16 + i] & temp;
    } else {
      shiftBit = 1;
    }
    if (shiftBit == 0) {
      displaySwapBuffer[1][16 + i] = (displaySwapBuffer[1][16 + i] << 1) & 0xfe;
    } else {
      shiftBit = 1;
      displaySwapBuffer[1][16 + i] = (displaySwapBuffer[1][16 + i] << 1) | 0x01;
    }
  }
  temp = (temp >> 1) & 0x7f;
  if (temp == 0x00) {
    temp = 0x80;
  }
}

// Fonction pour scanner les lignes
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

// Fonction pour envoyer des données
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

// Fonction pour afficher la matrice
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
void resetSmileyDisplay() {
  smileyDisplayed = false;
  shiftCount = 0;
}
void setup() {



  // Démarrer le capteur DHT
  dht.begin();
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("Temp: ");
  lcd.noDisplay();  // Ecran éteint au démarrage

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

  // Initialisation de la matrice
  changeDisplay(happySmiley);
  cleardisplay();
}

void loop() {
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
    cleardisplay();
    Serial.println("Pas de présence, écran éteint.");
}

  // Vérification si le capteur SGP30 peut mesurer
  if (!sgp.IAQmeasure()) {
    Serial.println("Erreur lors de la mesure de la qualité de l'air.");
    return;
  }

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
        if (pressed > 4) {
          pressed = 0;  // Remettre à 0 après le troisième état
        }
        if (pressed == 4) {
          resetSmileyDisplay();
        }
        lcd.clear();  // Effacer l'écran LCD pour afficher les nouvelles données
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
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.setCursor(0, 1);
    lcd.print(temperature);
    lcd.print(" C");
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
  } else if (pressed == 3){
    lcd.setCursor (0,0);
    lcd.print("heure et date");
  
  } else if (pressed == 4 && !smileyDisplayed) {
    for (int i = 0; i < 30; i++) {
      display(displaySwapBuffer);
    }
    displayWordCount = shiftCount / 16;
    calcShift();
    shiftCount++;
    if (shiftCount == (NUM_OF_WORD + DISPLAY_NUM_WORD) * 16) {
      smileyDisplayed = true;
      cleardisplay();
    }
  }
  // Mise à jour de l'état précédent du bouton
  lastButtonState = currentButtonState;
}

