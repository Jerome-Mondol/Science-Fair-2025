#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_PIN A0
#define FAN_RELAY 5

float temp_threshold = 32.0;
int soil_mois_threshold = 500;

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(FAN_RELAY, OUTPUT);
  digitalWrite(FAN_RELAY, LOW);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Intro screen
  display.clearDisplay();
  display.setTextSize(2); // Bigger text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Smart");
  display.println("Greenhouse");
  display.display();
  delay(2000);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soil = analogRead(SOIL_PIN);

  // Fan control
  if(temp > temp_threshold) digitalWrite(FAN_RELAY, HIGH);
  else digitalWrite(FAN_RELAY, LOW);

  // OLED display with bigger text
  display.clearDisplay();

  display.setTextSize(2); // Bigger text
  display.setCursor(0,0);
  display.print("T:"); display.print(temp,1); display.print("C");

  display.setCursor(0,20);
  display.print("H:"); display.print(hum,1); display.print("%");

  display.setCursor(0,40);
  display.print("S:"); display.print(soil>soil_mois_threshold?"Dry":"Wet");

  display.setCursor(64,40); // Fan status on the right
  display.print("F:"); display.print(digitalRead(FAN_RELAY)?"ON":"OFF");

  display.display();
  delay(1000);
}
