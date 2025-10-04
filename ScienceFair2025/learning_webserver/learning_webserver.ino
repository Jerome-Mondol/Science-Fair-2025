#include <ESP8266WiFi.h>

const char* ssid = "AFB 0.1";
const char* password = "nospoonishere";

const int IN1 = D1;
const int IN2 = D2;
const int IN3 = D3;
const int IN4 = D4;

WiFiServer server(80);

enum MotorState { STOPPED, FORWARD, BACKWARD, LEFT, RIGHT };
MotorState currentMotorState = STOPPED;

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.begin();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();
}

void loop() {
  WiFiClient client = server.available();

  if (!client) return;

  Serial.println("Client connected");
  String request = client.readStringUntil('\n');
  Serial.println("Request: " + request);

  if (request.indexOf("GET /forward") != -1) {
    forward();
    currentMotorState = FORWARD;
    respondText(client, "Moving Forward");
  } else if (request.indexOf("GET /backward") != -1) {
    backward();
    currentMotorState = BACKWARD;
    respondText(client, "Moving Backward");
  } else if (request.indexOf("GET /left") != -1) {
    left();
    currentMotorState = LEFT;
    respondText(client, "Turning Left");
  } else if (request.indexOf("GET /right") != -1) {
    right();
    currentMotorState = RIGHT;
    respondText(client, "Turning Right");
  } else if (request.indexOf("GET /stop") != -1) {
    stopMotors();
    currentMotorState = STOPPED;
    respondText(client, "Stopped");
  } else {
    // Serve main page
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><head><title>ESP8266 Motor Control</title></head><body>");
    client.println("<h2>Motor Control Panel</h2>");
    client.println("<p>Current state: <span id='state'>None</span></p>");
    client.println("<button onmousedown=\"sendCmd('forward')\" onmouseup=\"sendCmd('stop')\">Forward</button><br><br>");
    client.println("<button onmousedown=\"sendCmd('left')\" onmouseup=\"sendCmd('stop')\">Left</button>");
    client.println("<button onmousedown=\"sendCmd('right')\" onmouseup=\"sendCmd('stop')\">Right</button><br><br>");
    client.println("<button onmousedown=\"sendCmd('backward')\" onmouseup=\"sendCmd('stop')\">Backward</button><br><br>");
    client.println("<button onclick=\"sendCmd('stop')\">Stop</button>");

    client.println("<script>");
    client.println("function sendCmd(cmd) {");
    client.println(" fetch('/' + cmd).then(r => r.text()).then(t => document.getElementById('state').innerText = t);");
    client.println("}");
    client.println("</script>");

    client.println("</body></html>");
  }

  client.stop();
  Serial.println("Client disconnected");
}

void respondText(WiFiClient& client, String message) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println(message);
}

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

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  Serial.println("Motors stopped");
}
