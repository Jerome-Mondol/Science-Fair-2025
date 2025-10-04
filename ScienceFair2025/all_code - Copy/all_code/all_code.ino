#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== DHT =====
#define DHTPIN_GREEN 2 // Greenhouse / OLED / Fan
#define DHTPIN_HOME 6  // Home / LCD
#define DHTTYPE DHT11
DHT dhtGreen(DHTPIN_GREEN, DHTTYPE); // OLED
DHT dhtHome(DHTPIN_HOME, DHTTYPE);   // LCD

// ===== Rain sensor =====
#define RAIN_PIN 3

// ===== Servo =====
Servo my_servo;
int servoPos = 0;
int targetPos = 0;
const int servoMin = 0;
const int servoMax = 90;
const unsigned long servoStepDelay = 20;  // 20 ms per degree
unsigned long lastServoMove = 0;

// ===== LDR =====
#define LDR_PIN A1

// ===== Soil sensor & Fan & Motor =====
#define SOIL_PIN 10       // Digital soil sensor on pin 10
#define FAN_RELAY 5
#define MOTOR_PIN 7       // Motor relay pin
float temp_threshold = 32.0;

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== Timers =====
unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 2000; // 2s refresh for LCD & OLED

void setup() {
  Serial.begin(9600);

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Weather Station");
  delay(2000);
  lcd.clear();

  // OLED init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Smart Greenhouse");
  display.display();
  delay(2000);

  // DHT sensors
  dhtGreen.begin(); // OLED / Fan
  dhtHome.begin();  // LCD / Home

  // Pins
  pinMode(RAIN_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);   // Soil digital sensor
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  digitalWrite(FAN_RELAY, LOW);
  digitalWrite(MOTOR_PIN, LOW);

  // Servo
  my_servo.attach(4);
  my_servo.write(servoPos);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- Greenhouse DHT (OLED / Fan) ---
  float tempGreen = dhtGreen.readTemperature();
  float humGreen = dhtGreen.readHumidity();

  // Fan control
  if(tempGreen > temp_threshold) digitalWrite(FAN_RELAY, HIGH);
  else digitalWrite(FAN_RELAY, LOW);

  // --- Soil sensor control (MOTOR_PIN = 7) ---
  int soil_reading = digitalRead(SOIL_PIN);
  Serial.print("Soil: ");
  Serial.println(soil_reading);

  if(soil_reading == HIGH) { // Dry
    digitalWrite(MOTOR_PIN, HIGH);
  } else { // Wet
    digitalWrite(MOTOR_PIN, LOW);
  }

  // --- Home DHT (LCD) ---
  float tempHome = dhtHome.readTemperature();
  float humHome = dhtHome.readHumidity();

  // Rain + Servo target
  int rain = digitalRead(RAIN_PIN);
  if(rain == HIGH) targetPos = servoMax; // dry -> open (90°)
  else targetPos = servoMin;             // rain -> close (0°)

  // Smooth servo movement (every 20ms)
  if(currentMillis - lastServoMove >= servoStepDelay){
    if(servoPos < targetPos) servoPos++;
    else if(servoPos > targetPos) servoPos--;
    my_servo.write(servoPos);
    lastServoMove = currentMillis;
  }

  // --- Update Displays every 2s ---
  if(currentMillis - lastDisplayUpdate >= displayInterval){
    lastDisplayUpdate = currentMillis;

    // LCD display (home)
    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(tempHome);
    lcd.print((char)223);
    lcd.print("C ");

    int ldrValue = analogRead(LDR_PIN);
    String dayNight = (ldrValue < 400) ? "Day  " : "Night";
    lcd.setCursor(11,0);
    lcd.print("     "); // clear old
    lcd.setCursor(11,0);
    lcd.print(dayNight);

    lcd.setCursor(0,1);
    lcd.print("Hum:");
    lcd.print(humHome);
    lcd.print("% ");
    lcd.print(rain==LOW ? "Rain  " : "Dry ");

    // OLED display (greenhouse)
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print("T:"); display.print(tempGreen,1); display.print("C");
    display.setCursor(0,20);
    display.print("H:"); display.print(humGreen,1); display.print("%");
    display.setCursor(0,40);
    display.print("S:"); display.print(soil_reading==HIGH ? "Dry":"Wet");
    display.setCursor(64,40);
    display.print("F:"); display.print(digitalRead(FAN_RELAY)?"ON":"OFF");
    display.display();
  }
}
