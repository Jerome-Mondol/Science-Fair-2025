#include <Servo.h>

Servo my_servo;
#define rain_sensor 3  // digital pin for rain sensor

// Servo control variables
int servoPos = 0;       // current position
int targetPos = 0;      // target position
const int servoMin = 0;
const int servoMax = 90;
const unsigned long servoStepDelay = 20;  // ms between each servo step
unsigned long lastServoMove = 0;

void setup() {
  Serial.begin(9600);
  pinMode(rain_sensor, INPUT);
  my_servo.attach(4);
  my_servo.write(servoPos);  // start at 0°
}

void loop() {
  int sensor_state = digitalRead(rain_sensor);  // HIGH = wet, LOW = dry
  Serial.println(sensor_state);

  // Determine target position
  if (sensor_state == HIGH) {
    targetPos = servoMax;  // wet → open
  } else {
    targetPos = servoMin;  // dry → close
  }

  // --- Non-blocking smooth servo movement ---
  unsigned long currentMillis = millis();
  if (currentMillis - lastServoMove >= servoStepDelay) {
    if (servoPos < targetPos) servoPos++;
    else if (servoPos > targetPos) servoPos--;

    my_servo.write(servoPos);
    lastServoMove = currentMillis;
  }
}
