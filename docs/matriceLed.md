# Matrice LED 16x16
[Cette matrice](https://www.wiltronics.com.au/product/19744/dot-matrix-16x16-red-led-display-module-arduino-compatible/) de 16x16 (leds rouges) a très peu de documentation en ligne.
## Example de code pour afficher des smileys
```C++
// Définition des broches pour le contrôle des LED
const int LEDARRAY_D = 2;
const int LEDARRAY_C = 3;
const int LEDARRAY_B = 4;
const int LEDARRAY_A = 5;
const int LEDARRAY_G = 6;
const int LEDARRAY_DI = 7;
const int LEDARRAY_CLK = 8;
const int LEDARRAY_LAT = 9;

// Le nombre de mots et de caractères à afficher
const int DISPLAY_NUM_WORD = 2;
const int NUM_OF_WORD = 2; // 5 lettres pour "pluie"

unsigned char displayBuffer[8];
unsigned char displaySwapBuffer[DISPLAY_NUM_WORD][32] = { 0 };
bool shiftBit = 0;
bool flagShift = 0;
unsigned char timerCount = 0;
unsigned char temp = 0x80;
unsigned char shiftCount = 0;
unsigned char displayWordCount = 0;
const char happySmiley[2][32] = {
//hAPPY sMILEY
0xF8, 0xE0, 0xC0, 0x86, 0x86,0x06,0x00, 0x20, 
0x30, 0x18, 0x8C, 0x87,0xC0, 0xE0, 0xF8, 0xFF, 
0x3F, 0x0F, 0x07, 0xC3, 0xC3, 0xC1, 0x01, 0x09, 
0x19, 0x31, 0x63, 0xC3, 0x07, 0x0F, 0x3F, 0xFF,
};

const char coldSmiley[2][32] = {
// COLD SMILEY 
0xFF, 0xFF, 0xE7, 0xDB, 0xFF, 0xE7, 0xE7, 0xFF,
0xFF, 0xC0, 0xD6, 0xDA, 0xC0, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xCF, 0xB7, 0xFF, 0xCF, 0xCF, 0xFF,
0xFF, 0x07, 0xD7, 0xB7, 0x07, 0xFF, 0xFF, 0xFF,
};

const char sunSmiley[2][32] = {
// SMILEY SUN
0xFF, 0xFF, 0x02, 0x80, 0x82, 0x87, 0xFF, 0xFF,
0x3F, 0xDF, 0xCF, 0xEF, 0xF0, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0x07, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF,
0xE7, 0xCF, 0x9F, 0xBF, 0x7F, 0xFF, 0xFF, 0xFF,
};
const char warning[2][32] = {
// WARNING SIGN
0xFF, 0xFE, 0xFD, 0xFB, 0xFA, 0xF6, 0xF6, 0xE6,
0xEE, 0xEF, 0xCE, 0xDE, 0x9F, 0xBF, 0x80, 0xFF,
0x7F, 0x3F, 0xDF, 0xEF, 0x2F, 0x37, 0x37, 0x33, 
0x3B, 0xFB, 0x39, 0x3D, 0x03, 0x01, 0x00, 0xFF,
};

const char (*MESSAGE)[32];


void setup() {
  pinMode(LEDARRAY_D, OUTPUT);
  pinMode(LEDARRAY_C, OUTPUT);
  pinMode(LEDARRAY_B, OUTPUT);
  pinMode(LEDARRAY_A, OUTPUT);
  pinMode(LEDARRAY_G, OUTPUT);
  pinMode(LEDARRAY_DI, OUTPUT);
  pinMode(LEDARRAY_CLK, OUTPUT);
  pinMode(LEDARRAY_LAT, OUTPUT);
  changeDisplay(happySmiley);
  cleardisplay();
}

void loop() {
  for (int i = 0; i < 30; i++) {
    display(displaySwapBuffer);
  }
  displayWordCount = shiftCount / 16;
  calcShift();
  shiftCount++;
  if (shiftCount == (NUM_OF_WORD + DISPLAY_NUM_WORD) * 16) {
    shiftCount = 0;
  }
}
void changeDisplay(const char (*newMessage)[32]) {
  // Met à jour le message que le tableau "MESSAGE" va contenir
  MESSAGE = newMessage; 
  cleardisplay();  // Efface l'écran avant d'afficher un nouveau message
}

void cleardisplay() {
  for (int j = 0; j < DISPLAY_NUM_WORD; j++) {
    for (int i = 0; i < 32; i++) {
      displaySwapBuffer[j][i] = 0xFF;
    }
  }
}


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
```