/**
 * AI-Enabled Quality Inspection Conveyor
 * STM32F103RB Motor Controller (with Sensors)
 * 
 * Serial    = USB (Debug via Serial Monitor)
 * espSerial = UART on PA10(RX), PA9(TX) -> Connected to ESP32
 */

// Create UART for ESP32 communication (RX=PA10, TX=PA9)
HardwareSerial espSerial(PA10, PA9);

// L298N Pin Definitions
const int ENA = PA0;   
const int IN1 = PA1;   
const int IN2 = PA4;   
const int IN3 = PB0;   
const int IN4 = PB1;   
const int ENB = PB10;  

// Sensor Pin Definitions
const int IR_PIN = PB12; 
const int TRIG_PIN = PB13;
const int ECHO_PIN = PB14;

int currentSpeed = 150;
bool isRunning = false;
bool autoStopEnabled = true;
unsigned long lastSensorRead = 0;
unsigned long lastSensorSend = 0;
long lastDistance = 999;
bool lastIR = false;

void setup() {
  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Sensor pins
  pinMode(IR_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  stopConveyor();

  Serial.begin(115200);
  espSerial.begin(115200);

  Serial.println("=== STM32 Conveyor Controller ===");
  Serial.println("Waiting for commands...");
}

void loop() {
  // ========= PRIORITY 1: READ COMMANDS (ALWAYS) =========
  // Check ESP32 commands FIRST, every single loop
  while (espSerial.available() > 0) {
    String command = espSerial.readStringUntil('\n');
    command.trim();
    if (command.length() > 0) {
      handleCommand(command);
    }
  }

  // Check USB Serial commands too
  while (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.length() > 0) {
      handleCommand(command);
    }
  }

  // ========= PRIORITY 2: READ SENSORS (every 300ms) =========
  if (millis() - lastSensorRead > 300) {
    lastIR = (digitalRead(IR_PIN) == LOW);
    lastDistance = getDistance();
    lastSensorRead = millis();

    // Auto-stop if IR detects package
    if (isRunning && autoStopEnabled && lastIR) {
      stopConveyor();
      Serial.println("[AUTO] Package detected! Conveyor stopped.");
      espSerial.println("EVENT:PACKAGE_DETECTED");
    }
  }

  // ========= PRIORITY 3: SEND SENSOR DATA (every 1000ms) =========
  if (millis() - lastSensorSend > 1000) {
    String sensorData = "IR:" + String(lastIR) + " DIST:" + String(lastDistance);
    espSerial.println(sensorData);
    Serial.println(sensorData);
    lastSensorSend = millis();
  }
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 15000); // Reduced timeout to 15ms
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void handleCommand(String command) {
  Serial.print("[CMD] Received: '");
  Serial.print(command);
  Serial.println("'");

  if (command == "START") {
    startConveyor();
  } else if (command == "STOP") {
    stopConveyor();
  } else if (command.startsWith("S")) {
    int speed = command.substring(1).toInt();
    if (speed > 0) setSpeed(speed);
  } else if (command == "AUTOON") {
    autoStopEnabled = true;
    espSerial.println("MODE:AUTO_ON");
  } else if (command == "AUTOOFF") {
    autoStopEnabled = false;
    espSerial.println("MODE:AUTO_OFF");
  }
}

void startConveyor() {
  isRunning = true;
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  updateSpeed();
  Serial.println("[MOTOR] RUNNING at speed: " + String(currentSpeed));
  espSerial.println("STATUS:RUNNING");
}

void stopConveyor() {
  isRunning = false;
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  Serial.println("[MOTOR] STOPPED");
  espSerial.println("STATUS:STOPPED");
}

void setSpeed(int speed) {
  currentSpeed = constrain(speed, 0, 255);
  if (isRunning) updateSpeed();
  Serial.println("[SPEED] Set to: " + String(currentSpeed));
  espSerial.println("SPEED:" + String(currentSpeed));
}

void updateSpeed() {
  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}