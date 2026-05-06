#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ═══════════════════════════════════════════
//   KONFIGURASI - GANTI SESUAI WIFI ANDA
// ═══════════════════════════════════════════
const char* ssid     = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI_ANDA";

// ═══════════════════════════════════════════
//   PIN DEFINITION
// ═══════════════════════════════════════════
#define SERVO_LEFT_PIN   4   // GPIO4 → Servo Kiri
#define SERVO_RIGHT_PIN  5   // GPIO5 → Servo Kanan
#define OLED_SDA         6   // GPIO6 → SDA OLED
#define OLED_SCL         7   // GPIO7 → SCL OLED

// ═══════════════════════════════════════════
//   OLED CONFIG
// ═══════════════════════════════════════════
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDRESS  0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ═══════════════════════════════════════════
//   SERVO CONFIG
// ═══════════════════════════════════════════
Servo servoLeft;
Servo servoRight;

// Servo 360° - nilai microseconds
// 1500 = berhenti, 1000 = full CW, 2000 = full CCW
#define SERVO_STOP      1500
#define SERVO_FULL_FWD  1800
#define SERVO_FULL_BWD  1200
#define SERVO_SLOW_FWD  1600
#define SERVO_SLOW_BWD  1400

// ═══════════════════════════════════════════
//   WEB SERVER
// ═══════════════════════════════════════════
WebServer server(80);

// ═══════════════════════════════════════════
//   STATE
// ═══════════════════════════════════════════
bool isCute        = true;
unsigned long lastFaceChange = 0;
const unsigned long FACE_INTERVAL = 5000; // 5 detik

String currentMove = "stop";
String wifiIP      = "";

// ═══════════════════════════════════════════
//   WAJAH IMUT
// ═══════════════════════════════════════════
void drawCuteFace() {
  display.clearDisplay();

  // Mata kiri (bulat besar)
  display.fillCircle(38, 28, 12, SSD1306_WHITE);
  display.fillCircle(38, 28,  6, SSD1306_BLACK);
  display.fillCircle(34, 24,  3, SSD1306_WHITE); // kilap

  // Mata kanan (bulat besar)
  display.fillCircle(90, 28, 12, SSD1306_WHITE);
  display.fillCircle(90, 28,  6, SSD1306_BLACK);
  display.fillCircle(86, 24,  3, SSD1306_WHITE); // kilap

  // Pipi kiri
  display.fillCircle(20, 42, 6, SSD1306_WHITE);

  // Pipi kanan
  display.fillCircle(108, 42, 6, SSD1306_WHITE);

  // Mulut senyum (arc)
  for (int i = -20; i <= 20; i++) {
    int x = 64 + i;
    int y = 50 + (i * i) / 40;
    display.drawPixel(x, y, SSD1306_WHITE);
    display.drawPixel(x, y + 1, SSD1306_WHITE);
  }

  // Telinga kecil di atas
  display.fillRoundRect(20, 2, 16, 10, 4, SSD1306_WHITE);
  display.fillRoundRect(92, 2, 16, 10, 4, SSD1306_WHITE);

  display.display();
}

// ═══════════════════════════════════════════
//   WAJAH MARAH
// ═══════════════════════════════════════════
void drawAngryFace() {
  display.clearDisplay();

  // Alis marah kiri (diagonal)
  for (int i = 0; i < 3; i++) {
    display.drawLine(22, 12 + i, 48, 18 + i, SSD1306_WHITE);
  }

  // Alis marah kanan (diagonal)
  for (int i = 0; i < 3; i++) {
    display.drawLine(80, 18 + i, 106, 12 + i, SSD1306_WHITE);
  }

  // Mata kiri (menyipit)
  display.fillRoundRect(26, 22, 22, 14, 4, SSD1306_WHITE);
  display.fillCircle(37, 29, 5, SSD1306_BLACK);

  // Mata kanan (menyipit)
  display.fillRoundRect(80, 22, 22, 14, 4, SSD1306_WHITE);
  display.fillCircle(91, 29, 5, SSD1306_BLACK);

  // Mulut cemberut
  for (int i = -18; i <= 18; i++) {
    int x = 64 + i;
    int y = 54 - (i * i) / 36;
    display.drawPixel(x, y, SSD1306_WHITE);
    display.drawPixel(x, y + 1, SSD1306_WHITE);
  }

  // Garis kerutan dahi
  display.drawLine(50, 8, 54, 14, SSD1306_WHITE);
  display.drawLine(74, 8, 78, 14, SSD1306_WHITE);

  display.display();
}

// ═══════════════════════════════════════════
//   TAMPILAN STATUS (saat bergerak)
// ═══════════════════════════════════════════
void showStatusOverlay(String status) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print(status);
  display.display();
}

// ═══════════════════════════════════════════
//   KONTROL SERVO
// ═══════════════════════════════════════════
void moveForward() {
  // Servo kiri & kanan berlawanan karena posisi terbalik
  servoLeft.writeMicroseconds(SERVO_FULL_FWD);
  servoRight.writeMicroseconds(SERVO_FULL_BWD);
  currentMove = "maju";
}

void moveBackward() {
  servoLeft.writeMicroseconds(SERVO_FULL_BWD);
  servoRight.writeMicroseconds(SERVO_FULL_FWD);
  currentMove = "mundur";
}

void turnLeft() {
  servoLeft.writeMicroseconds(SERVO_FULL_BWD);
  servoRight.writeMicroseconds(SERVO_FULL_BWD);
  currentMove = "kiri";
}

void turnRight() {
  servoLeft.writeMicroseconds(SERVO_FULL_FWD);
  servoRight.writeMicroseconds(SERVO_FULL_FWD);
  currentMove = "kanan";
}

void stopMotors() {
  servoLeft.writeMicroseconds(SERVO_STOP);
  servoRight.writeMicroseconds(SERVO_STOP);
  currentMove = "stop";
}

// ═══════════════════════════════════════════
//   WEB PAGE HTML (controller HP)
// ═══════════════════════════════════════════
String getHTML() {
  String html = R"rawhtml(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Mochi Robot</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Nunito:wght@400;700;900&display=swap');
  :root {
    --bg: #1a1a2e;
    --card: #16213e;
    --accent: #e94560;
    --accent2: #f5a623;
    --green: #0f9b58;
    --text: #eaeaea;
    --btn-shadow: 0 6px 0px rgba(0,0,0,0.4);
  }
  * { box-sizing: border-box; margin: 0; padding: 0; -webkit-tap-highlight-color: transparent; }
  body {
    background: var(--bg);
    font-family: 'Nunito', sans-serif;
    color: var(--text);
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 16px;
    gap: 16px;
  }
  header {
    text-align: center;
    padding: 12px 0 4px;
  }
  header h1 {
    font-size: 2rem;
    font-weight: 900;
    color: #fff;
    letter-spacing: -1px;
  }
  header h1 span { color: var(--accent); }
  header p { font-size: 0.8rem; color: #888; margin-top: 2px; }

  .status-card {
    background: var(--card);
    border-radius: 16px;
    padding: 10px 20px;
    display: flex;
    align-items: center;
    gap: 10px;
    width: 100%;
    max-width: 340px;
    border: 1px solid #ffffff10;
  }
  .dot {
    width: 10px; height: 10px;
    border-radius: 50%;
    background: var(--green);
    box-shadow: 0 0 8px var(--green);
    animation: pulse 1.5s infinite;
  }
  @keyframes pulse {
    0%,100% { opacity: 1; } 50% { opacity: 0.4; }
  }
  .status-text { font-size: 0.85rem; }
  .status-text span { font-weight: 700; color: var(--accent2); }

  /* Face display */
  .face-card {
    background: var(--card);
    border-radius: 20px;
    padding: 16px;
    width: 100%;
    max-width: 340px;
    text-align: center;
    border: 1px solid #ffffff10;
  }
  .face-label {
    font-size: 0.75rem;
    color: #666;
    margin-bottom: 8px;
    text-transform: uppercase;
    letter-spacing: 2px;
  }
  .face-display {
    font-size: 3rem;
    line-height: 1;
    margin: 4px 0;
  }
  .face-name {
    font-size: 0.9rem;
    font-weight: 700;
    color: var(--accent2);
    margin-top: 4px;
  }

  /* D-PAD */
  .dpad-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 8px;
    width: 100%;
    max-width: 340px;
  }
  .dpad-label {
    font-size: 0.75rem;
    color: #666;
    text-transform: uppercase;
    letter-spacing: 2px;
    align-self: flex-start;
  }
  .dpad {
    display: grid;
    grid-template-columns: repeat(3, 90px);
    grid-template-rows: repeat(3, 90px);
    gap: 8px;
  }
  .btn {
    border: none;
    border-radius: 14px;
    font-family: 'Nunito', sans-serif;
    font-weight: 900;
    font-size: 1.6rem;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: transform 0.08s, box-shadow 0.08s;
    user-select: none;
    -webkit-user-select: none;
  }
  .btn:active {
    transform: translateY(4px);
    box-shadow: 0 2px 0 rgba(0,0,0,0.4) !important;
  }
  .btn-dir {
    background: #0f3460;
    color: #fff;
    box-shadow: var(--btn-shadow);
    border: 1px solid #ffffff15;
  }
  .btn-dir:active, .btn-dir.active {
    background: var(--accent);
  }
  .btn-stop {
    background: var(--accent);
    color: #fff;
    box-shadow: 0 6px 0 #8a1530;
    font-size: 1rem;
    grid-column: 2;
    grid-row: 2;
  }
  .btn-stop:active {
    background: #c23050;
  }
  .btn-empty { visibility: hidden; }

  /* Move status */
  .move-badge {
    background: var(--card);
    border-radius: 12px;
    padding: 8px 20px;
    font-size: 0.85rem;
    border: 1px solid #ffffff10;
    width: 100%;
    max-width: 340px;
    text-align: center;
  }
  .move-badge span {
    font-weight: 700;
    color: var(--accent);
    text-transform: uppercase;
  }

  footer {
    font-size: 0.7rem;
    color: #444;
    margin-top: auto;
    padding-bottom: 8px;
  }
</style>
</head>
<body>

<header>
  <h1>🤖 Mochi<span>Bot</span></h1>
  <p>Robot Controller</p>
</header>

<div class="status-card">
  <div class="dot"></div>
  <div class="status-text">Terhubung · IP: <span>)rawhtml";
  html += wifiIP;
  html += R"rawhtml(</span></div>
</div>

<div class="face-card">
  <div class="face-label">Ekspresi OLED</div>
  <div class="face-display" id="faceEmoji">🥰</div>
  <div class="face-name" id="faceName">Imut</div>
</div>

<div class="dpad-wrap">
  <div class="dpad-label">Kontrol Gerak</div>
  <div class="dpad">
    <div class="btn-empty"></div>
    <button class="btn btn-dir" id="btn-fwd"
      ontouchstart="send('forward')" ontouchend="send('stop')"
      onmousedown="send('forward')"  onmouseup="send('stop')">▲</button>
    <div class="btn-empty"></div>

    <button class="btn btn-dir" id="btn-left"
      ontouchstart="send('left')"    ontouchend="send('stop')"
      onmousedown="send('left')"     onmouseup="send('stop')">◀</button>
    <button class="btn btn-stop" id="btn-stop"
      ontouchstart="send('stop')"    ontouchend="send('stop')"
      onmousedown="send('stop')">⏹</button>
    <button class="btn btn-dir" id="btn-right"
      ontouchstart="send('right')"   ontouchend="send('stop')"
      onmousedown="send('right')"    onmouseup="send('stop')">▶</button>

    <div class="btn-empty"></div>
    <button class="btn btn-dir" id="btn-bwd"
      ontouchstart="send('backward')" ontouchend="send('stop')"
      onmousedown="send('backward')"  onmouseup="send('stop')">▼</button>
    <div class="btn-empty"></div>
  </div>
</div>

<div class="move-badge">
  Status: <span id="moveStatus">BERHENTI</span>
</div>

<footer>Robot Mochi · ESP32-C3</footer>

<script>
const moveLabel = {
  forward: 'MAJU', backward: 'MUNDUR',
  left: 'BELOK KIRI', right: 'BELOK KANAN', stop: 'BERHENTI'
};

function send(cmd) {
  document.getElementById('moveStatus').textContent = moveLabel[cmd] || cmd.toUpperCase();
  fetch('/cmd?action=' + cmd).catch(()=>{});
}

// Update face emoji setiap 5 detik sesuai ESP32
let cute = true;
function updateFace() {
  const emoji = document.getElementById('faceEmoji');
  const name  = document.getElementById('faceName');
  if (cute) {
    emoji.textContent = '🥰';
    name.textContent  = 'Imut';
  } else {
    emoji.textContent = '😡';
    name.textContent  = 'Marah';
  }
  cute = !cute;
}
setInterval(updateFace, 5000);
</script>
</body>
</html>
)rawhtml";
  return html;
}

// ═══════════════════════════════════════════
//   WEB SERVER ROUTES
// ═══════════════════════════════════════════
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleCmd() {
  if (server.hasArg("action")) {
    String action = server.arg("action");
    if      (action == "forward")  moveForward();
    else if (action == "backward") moveBackward();
    else if (action == "left")     turnLeft();
    else if (action == "right")    turnRight();
    else                           stopMotors();
  }
  server.send(200, "text/plain", "OK");
}

// ═══════════════════════════════════════════
//   OLED SPLASH SCREEN
// ═══════════════════════════════════════════
void showSplash() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(20, 10);
  display.print("MOCHI");
  display.setTextSize(1);
  display.setCursor(24, 32);
  display.print("Connecting WiFi");
  display.setCursor(30, 46);
  display.print("Please wait...");
  display.display();
}

void showConnected() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("WiFi Connected!");
  display.setCursor(0, 14);
  display.print("IP:");
  display.print(wifiIP);
  display.setCursor(0, 30);
  display.print("Buka di browser HP");
  display.setCursor(0, 44);
  display.print("untuk kontrol robot");
  display.display();
  delay(3000);
}

// ═══════════════════════════════════════════
//   SETUP
// ═══════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // Init I2C untuk OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED tidak terdeteksi! Cek wiring.");
    while (true); // berhenti kalau OLED gagal
  }
  showSplash();

  // Init Servo
  servoLeft.attach(SERVO_LEFT_PIN, 500, 2500);
  servoRight.attach(SERVO_RIGHT_PIN, 500, 2500);
  stopMotors();

  // Konek WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiIP = WiFi.localIP().toString();
    Serial.println("\nWiFi terhubung! IP: " + wifiIP);
    showConnected();
  } else {
    Serial.println("\nGagal konek WiFi! Cek SSID/password.");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi GAGAL!");
    display.setCursor(0, 14);
    display.print("Cek SSID/password");
    display.display();
  }

  // Setup web server routes
  server.on("/",    handleRoot);
  server.on("/cmd", handleCmd);
  server.begin();
  Serial.println("Web server berjalan.");
}

// ═══════════════════════════════════════════
//   LOOP
// ═══════════════════════════════════════════
void loop() {
  // Handle web requests
  server.handleClient();

  // Ganti ekspresi wajah setiap 5 detik
  unsigned long now = millis();
  if (now - lastFaceChange >= FACE_INTERVAL) {
    lastFaceChange = now;
    isCute = !isCute;
    if (isCute) drawCuteFace();
    else        drawAngryFace();

    // Tampilkan status gerak di baris bawah OLED
    // (sedikit terlambat agar animasi wajah terlihat dulu)
  }

  // Overlay status gerak di OLED
  if (currentMove != "stop") {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.fillRect(0, 56, 128, 8, SSD1306_BLACK);
    display.setCursor(0, 56);
    display.print(">> " + currentMove);
    display.display();
  }
}
