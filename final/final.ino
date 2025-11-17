#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>



WebServer server(80);
WebSocketsServer webSocket(81);

const char* ssid = "ESP32-Control";
const char* password = "12345678";

// Motor pins
const int IN1 = 25;
const int IN2 = 26;

// Servo
Servo myservo;
int SERVO_PIN = 13;

// HTML UI
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>GyroBalace Motorcycle</title>

<style>
    body {
        background: #e9edf1;
        font-family: "Inter", -apple-system, BlinkMacSystemFont, sans-serif;
        margin: 0;
        padding: 25px;
        display: flex;
        justify-content: center;
        color: #333;
    }

    .card {
        width: 100%;
        max-width: 430px;
        padding: 30px;
        background: #e9edf1;
        border-radius: 30px;
        box-shadow:
            8px 8px 16px #c5c9cc,
            -8px -8px 16px #ffffff;
        text-align: center;
    }

    h2 {
        margin-bottom: 25px;
        font-size: 24px;
        font-weight: 600;
    }

    .btn {
        width: 100%;
        padding: 18px;
        margin: 10px 0;
        border-radius: 20px;
        background: #e9edf1;
        border: none;
        font-size: 18px;
        font-weight: 600;
        cursor: pointer;
        transition: all 0.15s ease;

        box-shadow:
            6px 6px 12px #c5c9cc,
            -6px -6px 12px #ffffff;
    }

    .btn:active {
        box-shadow:
            inset 6px 6px 12px #c5c9cc,
            inset -6px -6px 12px #ffffff;
        transform: scale(0.98);
    }

    /* สีเฉพาะ */
    .forward { color: #2e7d32; }
    .back    { color: #c62828; }
    .stop    { color: #424242; }

    .steer-grid {
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        gap: 12px;
        margin: 8px 0 18px;
    }

</style>
</head>

<body>

<div class="card">
    <h2>GyroBalce</h2>

    <button class="btn forward" onclick="sendCmd('F')">Forward</button>

    <div class="steer-grid">
        <button class="btn" onclick="sendCmd('L')">Left</button>
        <button class="btn" onclick="sendCmd('C')">Center</button>
        <button class="btn" onclick="sendCmd('R')">Right</button>
    </div>

    <button class="btn back" onclick="sendCmd('B')">Backward</button>
    <button class="btn stop" onclick="sendCmd('S')">STOP</button>
</div>

<script>
let ws = new WebSocket("ws://" + location.hostname + ":81/");
function sendCmd(cmd){
    ws.send(cmd);
}
</script>

</body>
</html>
)rawliteral";


// ------- WebSocket Event -------
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(type == WStype_TEXT) {
    String cmd = (char*)payload;

    // Motor control
    if (cmd == "F") { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); }
    if (cmd == "B") { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); }
    if (cmd == "S") { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW); }

    // Servo myservo
    if (cmd == "L") myservo.write(45);    // เลี้ยวซ้าย
    if (cmd == "R") myservo.write(135);   // เลี้ยวขวา
    if (cmd == "C") myservo.write(90);    // กลับมาตรง
  }
}

// ------- Web server -------
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Servo setup
  myservo.attach(SERVO_PIN);
  myservo.write(90); // เริ่มต้นตรงกลาง

  // Start WiFi AP
  WiFi.softAP(ssid, password);
  Serial.println("AP Ready");

  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
