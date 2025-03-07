
# Capteur de présence


https://www.moussasoft.com/capteur-de-mouvement-pir-hc-sr501/

```c++
// Define the pins

const int ledPin = 4;
const int motionSensor = 2;

void setup() {
  Serial.begin(9600);
pinMode(motionSensor, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
int sensorValue = digitalRead(motionSensor);
if (sensorValue == HIGH){
  Serial.println("allumé");
  digitalWrite(ledPin, HIGH);
  delay(1000);
} else {
    Serial.println("Etaint");
  digitalWrite(ledPin, LOW);
}
}
```
![Description de l'image](/docs/assets/motion.jpg) 