#include <Servo.h>

Servo myservo;
void setup() {
  myservo.attach(4);
}

void loop() {
  myservo.write(0);   // close
}
