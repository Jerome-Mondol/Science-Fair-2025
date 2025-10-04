#include <ESP8266WiFi.h>

const char* ssid = "CaptainCar";
const char* password = "12345678";

WiFiServer server(80);

const int IN1 = D1;
const int IN2 = D2;
const int IN3 = D3;
const int IN4 = D4;

const int trigPin = D7;
const int echoPin = D8;

const int SAMPLE_COUNT = 5;
int distanceSamples[SAMPLE_COUNT];
int sampleIndex = 0;

bool obstacleDetected = false;
int avgDistanceCm = -1;

// Motor states
enum MotorState { STOPPED, FORWARD, BACKWARD, LEFT, RIGHT };
MotorState currentMotorState = STOPPED;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("🚀 CaptainBot booting...");

  WiFi.softAP(ssid, password);
  Serial.print("📶 AP Ready: ");
  Serial.println(WiFi.softAPIP());

  server.begin();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  stopMotors();

  // Initialize samples array with -1 (invalid)
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    distanceSamples[i] = -1;
  }
}

void loop() {
  // Take new reading and store in circular buffer
  distanceSamples[sampleIndex] = getDistance();
  sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;

  // Calculate average distance ignoring invalid samples
  avgDistanceCm = getAverageDistance();

  if (avgDistanceCm == -1) {
    Serial.println("⚠️ Warning: No valid distance reading");
    obstacleDetected = false;  // treat no reading as no obstacle
  } else {
    Serial.print("📏 Avg Distance: ");
    Serial.print(avgDistanceCm);
    Serial.println(" cm");

    obstacleDetected = (avgDistanceCm > 0 && avgDistanceCm < 20);
  }

  WiFiClient client = server.available();
  if (client) {
    while (!client.available()) {
      delay(1);
    }
    String req = client.readStringUntil('\r');
    client.flush();
    Serial.println("Request: " + req);
    handleRequest(req);
  }

  if (obstacleDetected) {
    stopMotors();
    currentMotorState = STOPPED;  // Force stop state
    Serial.println("🛑 Obstacle detected! Stopping motors.");
  } else {
    // Run motors according to current state
    switch (currentMotorState) {
      case FORWARD: forward(); break;
      case BACKWARD: backward(); break;
      case LEFT: left(); break;
      case RIGHT: right(); break;
      case STOPPED: stopMotors(); break;
    }
  }

  // Serve control page only if client connected
  if (client) {
    serveControlPage(client);
  }
}

// Function to calculate average ignoring invalid (-1) readings
int getAverageDistance() {
  int sum = 0;
  int count = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    if (distanceSamples[i] != -1) {
      sum += distanceSamples[i];
      count++;
    }
  }
  if (count == 0) return -1; // no valid samples
  return sum / count;
}

int getDistance() {
  long duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  int distance = duration * 0.034 / 2;

  // Validate distance
  if (duration == 0 || distance <= 0 || distance > 400) {
    return -1;
  }
  return distance;
}

void handleRequest(String req) {
  if (req.indexOf("/forward") != -1) currentMotorState = FORWARD;
  else if (req.indexOf("/backward") != -1) currentMotorState = BACKWARD;
  else if (req.indexOf("/left") != -1) currentMotorState = LEFT;
  else if (req.indexOf("/right") != -1) currentMotorState = RIGHT;
  else if (req.indexOf("/stop") != -1) currentMotorState = STOPPED;
}

void serveControlPage(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<title>Captain's WiFi Car</title><style>");
  client.println("body { background:#121212; color:#eee; font-family:Arial; text-align:center; }");
  client.println("button { font-size:1.5rem; margin:10px; padding:15px 30px; border:none; border-radius:10px; background:#00bb88; color:#fff; }");
  client.println("button:active { background:#007755; }");
  client.println(".row { display:flex; justify-content:center; flex-wrap:wrap; }");
  client.println("</style></head><body>");
  client.println("<h2>🚗 Captain's WiFi Car</h2>");
  client.println("<p>📏 Distance: " + String(avgDistanceCm == -1 ? 0 : avgDistanceCm) + " cm</p>");

  client.println("<div class='row'><button onmousedown=\"send('forward')\" onmouseup=\"send('stop')\" onmouseleave=\"send('stop')\">Forward</button></div>");
  client.println("<div class='row'>");
  client.println("<button onmousedown=\"send('left')\" onmouseup=\"send('stop')\" onmouseleave=\"send('stop')\">Left</button>");
  client.println("<button onmousedown=\"send('stop')\">Stop</button>");
  client.println("<button onmousedown=\"send('right')\" onmouseup=\"send('stop')\" onmouseleave=\"send('stop')\">Right</button>");
  client.println("</div>");
  client.println("<div class='row'><button onmousedown=\"send('backward')\" onmouseup=\"send('stop')\" onmouseleave=\"send('stop')\">Backward</button></div>");

  // JS to send repeated commands while holding button
  client.println("<script>");
  client.println("let intervalId;");
  client.println("function send(cmd) {");
  client.println("  if (cmd === 'stop') {");
  client.println("    clearInterval(intervalId);");
  client.println("    fetch('/stop');");
  client.println("    return;");
  client.println("  }");
  client.println("  clearInterval(intervalId);");
  client.println("  intervalId = setInterval(() => fetch('/' + cmd), 150);");
  client.println("}");
  client.println("</script>");

  client.println("</body></html>");
}

void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
