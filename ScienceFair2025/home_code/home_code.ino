#include <Servo.h>

Servo myservo;
void setup() {
  myservo.attach(4);
}

void loop() {
  myservo.write(0);   // close
  delay(2000);
  myservo.write(90);  // open
  delay(2000);
}
