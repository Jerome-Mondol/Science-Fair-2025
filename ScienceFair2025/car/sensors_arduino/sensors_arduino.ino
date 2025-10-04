#include <Servo.h>

Servo myservo_plough;
Servo myservo_seed;

int relayPin = 7;

void setup() {
  Serial.begin(9600);
  myservo_plough.attach(2);
  myservo_plough.write(90);
  myservo_seed.attach(3);
  myservo_seed.write(180);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void loop() {
  digitalWrite(relayPin, HIGH);
  delay(1500);

  myservo_plough.write(0);
  delay(300);
  myservo_plough.write(90);
  delay(300);

  myservo_seed.write(0);
  delay(300);
  myservo_seed.write(180);
  delay(300);

  digitalWrite(relayPin, LOW);

  if (digitalRead(relayPin) == HIGH) {
    digitalWrite(relayPin, LOW);
  }

  delay(1000);
}
