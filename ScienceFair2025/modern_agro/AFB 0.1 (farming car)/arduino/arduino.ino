#include <Servo.h>

Servo myservo_plough;
Servo myservo_seed;

int relayPin = 7;

void setup() {
  Serial.begin(9600);

  myservo_plough.attach(2);
  myservo_seed.attach(3);

  myservo_plough.write(0);
  myservo_seed.write(0);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void loop() {
  digitalWrite(relayPin, HIGH);
  delay(100);

  for (int pos = 0; pos <= 25; pos += 5) {
    myservo_plough.write(pos);
    myservo_seed.write(pos);
    delay(20);
  }

  for (int pos = 25; pos >= 0; pos -= 5) {
    myservo_plough.write(pos);
    myservo_seed.write(pos);
    delay(20);
  }

  digitalWrite(relayPin, LOW);
  delay(300);
}
