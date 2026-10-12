#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

/* 
 * PROJECT: AI-Enabled Quality Inspection 
 * COMPONENT: ESP32 Gateway & Dashboard
 */

// --- Network Settings ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- Pins ---
#define RXD2 16
#define TXD2 17

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Global State
String conveyorStatus = "STOPPED";
int currentSpeed = 0;
String inspectionResult = "READY";
bool conveyorStoppedByAI = false;  // Track if AI stopped the conveyor
int goodCount = 0;                 // Count consecutive GOOD after damage

// --- HTML Dashboard (Industrial Theme) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Industrial Quality Control Dashboard</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: rgba(30, 41, 59, 0.7);
            --accent-cyan: #22d3ee;
            --accent-orange: #fb923c;
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
            --success: #4ade80;
            --danger: #f87171;
        }

        * { margin: 0; padding: 0; box-sizing: border-box; font-family: 'Inter', sans-serif; }
        body { background-color: var(--bg-color); color: var(--text-primary); overflow-x: hidden; }

        header { padding: 2rem; border-bottom: 1px solid rgba(255, 255, 255, 0.1); display: flex; justify-content: space-between; align-items: center; background: rgba(15, 23, 42, 0.8); backdrop-filter: blur(10px); position: sticky; top: 0; z-index: 100; }
        h1 { font-size: 1.5rem; font-weight: 700; background: linear-gradient(to right, #22d3ee, #818cf8); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }

        .container { padding: 2rem; max-width: 1400px; margin: 0 auto; display: grid; grid-template-columns: 2fr 1fr; gap: 2rem; }

        .card { background: var(--card-bg); backdrop-filter: blur(16px); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 1.5rem; padding: 1.5rem; box-shadow: 0 10px 30px rgba(0,0,0,0.5); }

        /* Camera Grid */
        .camera-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 1rem; margin-bottom: 2rem; }
        .camera-view { background: #000; border-radius: 1rem; aspect-ratio: 16/9; position: relative; overflow: hidden; border: 2px solid #334155; }
        .camera-view img { width: 100%; height: 100%; object-fit: cover; }
        .camera-label { position: absolute; top: 1rem; left: 1rem; background: rgba(0,0,0,0.6); padding: 0.2rem 0.8rem; border-radius: 0.5rem; font-size: 0.8rem; border: 1px solid var(--accent-cyan); }

        /* Controls */
        .controls { display: flex; gap: 1rem; margin-top: 1rem; }
        button { flex: 1; padding: 1rem; border: none; border-radius: 0.8rem; font-weight: 600; cursor: pointer; transition: 0.3s; }
        .btn-start { background: var(--success); color: #000; }
        .btn-stop { background: var(--danger); color: #fff; }
        button:hover { opacity: 0.8; transform: translateY(-2px); }

        .status-indicator {
            width: 40px; height: 40px; border-radius: 50%; display: flex; align-items: center; justify-content: center;
            font-size: 0.6rem; font-weight: 800; border: 2px solid #334155; background: #0f172a; color: #334155; transition: 0.3s;
        }
        .indicator-active { background: var(--accent-orange); color: #000; border-color: var(--accent-orange); box-shadow: 0 0 15px var(--accent-orange); }

        .slider-container { margin-top: 2rem; }
        input[type=range] { width: 100%; height: 6px; background: #334155; border-radius: 5px; outline: none; appearance: none; }
        input[type=range]::-webkit-slider-thumb { appearance: none; width: 20px; height: 20px; background: var(--accent-cyan); border-radius: 50%; cursor: pointer; border: 4px solid var(--bg-color); }

        /* Status & Logs */
        .status-badge { display: inline-block; padding: 0.5rem 1rem; border-radius: 2rem; font-size: 0.9rem; font-weight: 700; margin-top: 0.5rem; }
        .status-ok { background: rgba(74, 222, 128, 0.2); color: var(--success); }
        .status-fail { background: rgba(248, 113, 113, 0.2); color: var(--danger); }

        .log-container { height: 300px; overflow-y: auto; margin-top: 1rem; font-family: monospace; font-size: 0.85rem; color: var(--text-secondary); }
        .log-entry { padding: 0.5rem; border-bottom: 1px solid rgba(255,255,255,0.05); }
        
        @media (max-width: 1024px) { .container { grid-template-columns: 1fr; } }
    </style>
</head>
<body>
    <header>
        <h1>QUALITY INSPECTION SYSTEM <span style="font-size: 0.8rem; color: var(--text-secondary);">v2.0</span></h1>
                <div id="connection-status" style="color: var(--success);">● LIVE</div>
            </header>
        
            <div class="container">
                <!-- Main Panel: Vision & Controls -->
                <div>
                    <div class="card">
                        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 1.5rem;">
                            <h3>REAL-TIME VISION</h3>
                            <div style="display: flex; gap: 1rem;">
                                <div id="irStatus" class="status-indicator" title="IR Sensor">IR</div>
                                <div id="ultrasonicStatus" class="status-indicator" title="Ultrasonic">US</div>
                            </div>
                        </div>
                <div class="camera-grid">
                    <div class="camera-view">
                        <div class="camera-label">TOP VIEW</div>
                        <img id="camTop" src="" alt="Enter Phone IP below">
                    </div>
                    <div class="camera-view">
                        <div class="camera-label">SIDE VIEW</div>
                        <img id="camSide" src="" alt="Enter Phone IP below">
                    </div>
                </div>

                <div style="display:flex; gap:0.5rem; margin-bottom:0.5rem; align-items:center;">
                    <input type="text" id="camTopIP" placeholder="Top View Phone IP" 
                        style="flex:1; padding:0.6rem; border-radius:0.5rem; border:1px solid #334155; background:#1e293b; color:#f8fafc; font-size:0.85rem;">
                    <input type="text" id="camSideIP" placeholder="Side View Phone IP" 
                        style="flex:1; padding:0.6rem; border-radius:0.5rem; border:1px solid #334155; background:#1e293b; color:#f8fafc; font-size:0.85rem;">
                    <button onclick="connectCam()" style="flex:0; padding:0.6rem 1.2rem; background:#818cf8; color:#fff; border-radius:0.5rem; font-size:0.85rem;">
                        CONNECT
                    </button>
                </div>

                <div class="slider-container">
                    <div style="display: flex; justify-content: space-between; margin-bottom: 0.5rem;">
                        <span>Conveyor Speed</span>
                        <span id="speedVal">0%</span>
                    </div>
                    <input type="range" id="speedSlider" min="0" max="255" value="0" oninput="sendSpeed(this.value)">
                </div>

                <div class="controls">
                    <button class="btn-start" onclick="sendCommand('START')">INITIATE CONVEYOR</button>
                    <button class="btn-stop" onclick="sendCommand('STOP')">EMERGENCY STOP</button>
                </div>
            </div>
        </div>

        <!-- Sidebar: Analytics & Logs -->
        <div style="display: flex; flex-direction: column; gap: 2rem;">
            <div class="card">
                <h3>SYSTEM STATUS</h3>
                <div style="margin-top: 1.5rem;">
                    <div style="color: var(--text-secondary); font-size: 0.8rem; text-transform: uppercase;">Operation Mode</div>
                    <div id="opMode" style="font-size: 1.2rem; font-weight: 600; color: var(--accent-cyan);">AUTOMATIC</div>
                </div>
                <div style="margin-top: 1.5rem;">
                    <div style="color: var(--text-secondary); font-size: 0.8rem; text-transform: uppercase;">Package Health</div>
                    <div id="pkgHealth" class="status-badge status-ok">STATUS: GOOD</div>
                </div>
            </div>

            <div class="card" style="flex-grow: 1;">
                <h3>INSPECTION LOGS</h3>
                <div id="log" class="log-container">
                    <div class="log-entry">[00:00:00] System Ready. Waiting for connection...</div>
                </div>
            </div>
        </div>
    </div>

    <script>
        let socket;
        function initWebSocket() {
            socket = new WebSocket('ws://' + window.location.hostname + ':81/');
            socket.onopen = () => addLog('WebSocket connected to ESP32');
            socket.onclose = () => addLog('WebSocket disconnected');
            socket.onmessage = function(event) {
                const data = JSON.parse(event.data);
                if(data.type === 'status') {
                    document.getElementById('speedVal').innerText = Math.round((data.speed/255)*100) + '%';
                    document.getElementById('opMode').innerText = data.status;
                    
                    const ir = document.getElementById('irStatus');
                    const us = document.getElementById('ultrasonicStatus');
                    ir.className = 'status-indicator ' + (data.ir == 1 ? 'indicator-active' : '');
                    
                    if(data.dist) {
                        us.innerText = data.dist + 'cm';
                        us.className = 'status-indicator ' + (data.dist < 20 ? 'indicator-active' : '');
                        us.style.fontSize = '0.5rem';
                    }
                    
                    if(data.event === 'PACKAGE_DETECTED') {
                        addLog('ALERT: Package Detected (Auto-Stop Activated)');
                    }
                }
                if(data.type === 'inspection') {
                    const health = document.getElementById('pkgHealth');
                    health.innerText = 'STATUS: ' + data.result;
                    health.className = 'status-badge ' + (data.result === 'DAMAGED' ? 'status-fail' : 'status-ok');
                    addLog('Package Inspection: ' + data.result);
                    if(data.result === 'DAMAGED') {
                        addLog('⚠️ DAMAGED DETECTED! CONVEYOR AUTO-STOPPED!');
                        document.getElementById('opMode').innerText = 'DAMAGE HALT';
                    }
                }
            };
        }

        function sendCommand(cmd) {
            socket.send(JSON.stringify({type: 'command', value: cmd}));
            addLog('Sending command: ' + cmd);
        }

        function sendSpeed(val) {
            document.getElementById('speedVal').innerText = Math.round((val/255)*100) + '%';
            socket.send(JSON.stringify({type: 'speed', value: val}));
        }

        function addLog(msg) {
            const log = document.getElementById('log');
            const t = new Date().toLocaleTimeString();
            log.innerHTML = '<div class=\"log-entry\">[' + t + '] ' + msg + '</div>' + log.innerHTML;
        }

        function connectCam() {
            const topIP = document.getElementById('camTopIP').value.trim();
            const sideIP = document.getElementById('camSideIP').value.trim();
            if(topIP) {
                document.getElementById('camTop').src = 'http://' + topIP + ':8080/video';
                addLog('Top camera: ' + topIP);
            }
            if(sideIP) {
                document.getElementById('camSide').src = 'http://' + sideIP + ':8080/video';
                addLog('Side camera: ' + sideIP);
            }
            if(!topIP && !sideIP) { alert('Enter at least one phone IP!'); }
        }

        window.onload = () => { initWebSocket(); };
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // Communication with STM32

  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected. IP: " + WiFi.localIP().toString());

  // Server routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });
  server.begin();

  // WebSockets
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // Read status from STM32
  if (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    processSTM32Data(line);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.println("[WS] Client connected");
  }
  if (type == WStype_TEXT) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    
    String msgType = doc["type"];
    
    // Manual commands from dashboard buttons
    if (msgType == "command") {
      String val = doc["value"];
      Serial.println("[CMD->STM32] " + val);
      Serial2.println(val);
      if (val == "START") conveyorStoppedByAI = false;
    } 
    // Speed control from slider
    else if (msgType == "speed") {
      int val = doc["value"];
      Serial.println("[SPEED->STM32] S" + String(val));
      Serial2.println("S" + String(val));
    }
    // AI Inspection result from Python
    else if (msgType == "inspection") {
      String result = doc["result"];
      Serial.println("[AI] Inspection: " + result);
      
      // DAMAGED -> STOP conveyor
      if (result == "DAMAGED" && !conveyorStoppedByAI) {
        Serial.println("[AI->STM32] STOP (Damaged package!)");
        Serial2.println("STOP");
        conveyorStoppedByAI = true;
        goodCount = 0;
      }
      // GOOD after damage -> count and auto-restart
      else if (result == "GOOD" && conveyorStoppedByAI) {
        goodCount++;
        Serial.println("[AI] Good frame " + String(goodCount) + "/3");
        if (goodCount >= 3) {
          Serial.println("[AI->STM32] START (3x Good confirmed!)");
          Serial2.println("START");
          conveyorStoppedByAI = false;
          goodCount = 0;
        }
      }
      else if (result == "DAMAGED") {
        goodCount = 0;  // Reset good counter
      }
      
      // Broadcast inspection to all dashboard clients
      String output;
      serializeJson(doc, output);
      webSocket.broadcastTXT(output);
    }
  }
}

void processSTM32Data(String data) {
    data.trim();
    StaticJsonDocument<200> doc;
    doc["type"] = "status";

    if (data.startsWith("STATUS:")) {
        conveyorStatus = data.substring(7);
        doc["status"] = conveyorStatus;
    } else if (data.startsWith("SPEED:")) {
        currentSpeed = data.substring(6).toInt();
        doc["speed"] = currentSpeed;
    } else if (data.startsWith("IR:")) {
        // Simple manual parsing of "IR:1 DIST:15"
        doc["ir"] = data.substring(3, 4).toInt();
        if(data.indexOf("DIST:") != -1) {
            doc["dist"] = data.substring(data.indexOf("DIST:") + 6).toInt();
        }
    } else if (data.indexOf("PACKAGE_DETECTED") != -1) {
        doc["event"] = "PACKAGE_DETECTED";
        doc["status"] = "INSPECTION PAUSE";
    }

    String output;
    serializeJson(doc, output);
    webSocket.broadcastTXT(output);
}
