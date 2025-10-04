#define rainPin 3
#define pumpPin 7


void setup() {
  // put your setup code here, to run once:
  pinMode(rainPin, INPUT);
  pinMode(pumpPin, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int rain = digitalRead(rainPin);
  Serial.println(rain);
  if(rain == HIGH) digitalWrite(pumpPin, HIGH);
  else digitalWrite(pumpPin, LOW);
}




