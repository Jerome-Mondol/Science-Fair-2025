#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

bool isMovingForward = false;

const int IN1 = D1;
const int IN2 = D2;
const int IN3 = D3;
const int IN4 = D4;
const int ENA = D7;
const int ENB = D8;

const int trigPin = D6;
const int echoPin = D5;

ESP8266WebServer server(80);

const char* ssid = "AFB not working";
const char* password = "itisworking";

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Moving forward");
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Moving backward");
}

void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Turning left");
}

void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Turning right");
}

void stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

double obstacleDistance() {
  double readings[5];
  int validCount = 0;

  for (int i = 0; i < 5; i++) {
    long duration;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH, 20000);

    double dist = duration * 0.034 / 2;
    if (dist >= 2 && dist <= 400) {
      readings[validCount++] = dist;
    }

    delay(10);
  }

  if (validCount == 0) return -1.0;

  for (int i = 0; i < validCount - 1; i++) {
    for (int j = i + 1; j < validCount; j++) {
      if (readings[i] > readings[j]) {
        double temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }

  return readings[validCount / 2];
}

String htmlPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP Car Control</title>
<style>
  body {
    font-family: Arial, sans-serif;
    background-color: #f4f4f4;
    margin: 0;
    padding: 0;
    display: flex;
    flex-direction: column;
    align-items: center;
  }
  h1 {
    color: #333;
    margin: 20px 0;
    font-size: clamp(1.2rem, 5vw, 2rem);
  }
  .btn {
    width: clamp(120px, 30vw, 160px);
    height: clamp(50px, 12vw, 70px);
    margin: 8px;
    font-size: clamp(1rem, 4vw, 1.4rem);
    border: none;
    border-radius: 10px;
    cursor: pointer;
    background-color: #3498db;
    color: white;
    transition: background 0.2s, transform 0.1s;
    touch-action: manipulation;
  }
  .btn:active {
    transform: scale(0.95);
    background-color: #2980b9;
  }
  .row {
    display: flex;
    justify-content: center;
    flex-wrap: wrap;
    gap: 8px;
  }
</style>
</head>
<body>
  <h1>🚗 ESP Car Control</h1>

  <div class="row">
    <button class="btn" onmousedown="sendCmd('forward')" ontouchstart="sendCmd('forward')" onmouseup="sendCmd('stop')" ontouchend="sendCmd('stop')">Forward</button>
  </div>
  <div class="row">
    <button class="btn" onmousedown="sendCmd('left')" ontouchstart="sendCmd('left')" onmouseup="sendCmd('stop')" ontouchend="sendCmd('stop')">Left</button>
    <button class="btn" onmousedown="sendCmd('stop')" ontouchstart="sendCmd('stop')" onmouseup="sendCmd('stop')" ontouchend="sendCmd('stop')">Stop</button>
    <button class="btn" onmousedown="sendCmd('right')" ontouchstart="sendCmd('right')" onmouseup="sendCmd('stop')" ontouchend="sendCmd('stop')">Right</button>
  </div>
  <div class="row">
    <button class="btn" onmousedown="sendCmd('backward')" ontouchstart="sendCmd('backward')" onmouseup="sendCmd('stop')" ontouchend="sendCmd('stop')">Backward</button>
  </div>

<script>
  let lastCmdTime = 0;
  function sendCmd(dir) {
    const now = Date.now();
    if (now - lastCmdTime > 100) {
      fetch('/' + dir);
      lastCmdTime = now;
    }
  }
</script>

</body>
</html>
)rawliteral";
}

void handleRoot() { server.send(200, "text/html", htmlPage()); }

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  analogWrite(ENA, 150);
  analogWrite(ENB, 150);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
  server.on("/forward", [] () {
    double dist = obstacleDistance();
    if(dist > 0 && dist < 10) {
      stop();
      isMovingForward = false;
      Serial.print("Obstacle detected :");
      Serial.println(dist);
    } else {
      forward();
      isMovingForward = true;
      server.send(200, "text/html", "Moving forward");
    }
  });
  server.on("/backward", [] () {
    backward();
    server.send(200, "text/html", "Moving backward");
  });
  server.on("/left", [] () {
    left();
    server.send(200, "text/html", "Going left");
  });
  server.on("/right", [] () {
    right();
    server.send(200, "text/html", "Going right");
  });
  server.on("/stop", [] () {
    stop();
    isMovingForward = false;
    server.send(200, "text/html", "Motors stopped");
  });

  server.begin();
}
void loop() {
  server.handleClient();
  if(isMovingForward) {
    double dist = obstacleDistance();
      if(dist > 0 && dist < 20) {
        stop();
        isMovingForward = false;
        Serial.print("Obstacle detected :");
        Serial.println(dist);
      }
  }
}
