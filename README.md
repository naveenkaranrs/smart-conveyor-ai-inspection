# Smart Conveyor AI Quality Inspection System 🏭🤖

An intelligent, IoT-enabled industrial conveyor belt quality inspection system that integrates embedded microcontroller motor management (STM32 F103RB), edge wireless communication (ESP32 Gateway), and AI-powered computer vision defect detection.

---

## 🌟 Key Features

- **Embedded Motor Driver**: STM32 F103RB microcontroller managing L298N H-bridge for precise PWM motor speed and direction control.
- **Sensor Integration**: Infrared (IR) sensor for object detection and HC-SR04 Ultrasonic sensor for item distance/presence verification.
- **Wireless IoT Gateway**: ESP32 module acting as a gateway for sensor telemetry and relaying control commands.
- **AI Inspection Pipeline**: Computer vision vision-based quality control model inspecting item features in real-time.

---

## 📁 Repository Structure

```
.
├── ai_inspection/           # Computer vision defect & quality inspection models
├── conveyor_controller/     # STM32 F103RB main motor & sensor control firmware
├── esp32_gateway/           # ESP32 WiFi telemetry & remote command gateway
└── pin_connections.txt      # Hardware wiring guide & pin configuration matrix
```

---

## 🔌 Hardware Wiring Matrix

### 1. Motor Driver (STM32 to L298N)
| L298N Pin | STM32 Pin | Function |
| :--- | :--- | :--- |
| **ENA** | PA0 | Speed Control (PWM) - Motor Pair 1 |
| **IN1** | PA1 | Motor A Direction 1 |
| **IN2** | PA4 | Motor A Direction 2 |
| **IN3** | PB0 | Motor B Direction 1 |
| **IN4** | PB1 | Motor B Direction 2 |
| **ENB** | PB10 | Speed Control (PWM) - Motor Pair 2 |

### 2. Sensors
- **IR Sensor (Object Detection)**: VCC ➔ 5V | GND ➔ GND | OUT ➔ STM32 PB12
- **Ultrasonic Sensor (HC-SR04)**: VCC ➔ 5V | GND ➔ GND | TRIG ➔ STM32 PB13 | ECHO ➔ STM32 PB14

---

## 🚀 Getting Started

1. **Hardware Setup**: Wire the components according to `pin_connections.txt`.
2. **Flash Microcontrollers**:
   - Upload `conveyor_controller` firmware to STM32 F103RB via STM32CubeIDE / STM32duino.
   - Upload `esp32_gateway` code to the ESP32 using Arduino IDE.
3. **AI Inspection**: Run the Python AI inspection script in `ai_inspection/` with OpenCV and TensorFlow/PyTorch dependencies installed.

---

## 📄 License
This project is released under the [MIT License](LICENSE).
