# Smart Conveyor Dual-MCU Quality Inspection System 🏭🤖

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Target: STM32F103](https://img.shields.io/badge/Actuation-STM32F103-002B49?logo=stmicroelectronics)](https://www.st.com)
[![Gateway: ESP32](https://img.shields.io/badge/Gateway-ESP32-E7352C?logo=espressif)](https://www.espressif.com)

An automated industrial quality control conveyor belt prototype combining dual-microcontroller embedded control with real-time computer vision defect inspection.

---

## 🏗️ Hardware Architecture & Inter-MCU Bridge

```
   ┌─────────────────────────────────────────────────────────┐
   │               HOST PC (Computer Vision Node)            │
   │  OpenCV Canny Edge Density & Contour Irregularity Model │
   └────────────────────────────┬────────────────────────────┘
                                │ WebSocket (Port 81)
                                ▼
   ┌─────────────────────────────────────────────────────────┐
   │             ESP32 WIRELESS GATEWAY & DASHBOARD          │
   │  - Hosted HTTP Web Dashboard & WebSocket Server         │
   │  - Auto-Stop/Resume State Machine                       │
   └────────────────────────────┬────────────────────────────┘
                                │ Hardware UART (115200 Baud)
                                ▼
   ┌─────────────────────────────────────────────────────────┐
   │              STM32F103RB CONVEYOR CONTROLLER            │
   │  - Hardware Timers: PWM Speed Modulation (L298N Driver) │
   │  - Sensors: PB12 (IR Proximity), PB13/PB14 (Ultrasonic) │
   └─────────────────────────────────────────────────────────┘
```

---

## 🔌 Hardware Interconnects

### 1. STM32F103RB to L298N Motor Driver
| Function | STM32 Pin | L298N Driver | Description |
| :--- | :---: | :---: | :--- |
| **PWM Channel A** | PA0 (TIM2_CH1) | ENA | Front motor pair speed regulation |
| **Direction IN1** | PA1 | IN1 | Channel A forward logic |
| **Direction IN2** | PA4 | IN2 | Channel A reverse logic |
| **Direction IN3** | PB0 | IN3 | Channel B forward logic |
| **Direction IN4** | PB1 | IN4 | Channel B reverse logic |
| **PWM Channel B** | PB10 (TIM2_CH3)| ENB | Rear motor pair speed regulation |

### 2. Inter-MCU UART Telemetry Bus
- **STM32 USART1 (PA9 TX)** ──► **ESP32 GPIO16 (RX2)**
- **STM32 USART1 (PA10 RX)** ◄── **ESP32 GPIO17 (TX2)**
- **Common Signal Ground**

---

## 🔍 Vision Inspection Pipeline & Technical Details

- **Defect Detection Methodology:** Uses computer vision edge density analysis (Canny thresholding `[50, 150]`) and contour circularity ($4\pi \cdot \text{Area} / \text{Perimeter}^2$) in Python/OpenCV.
- **Fail-Safe Mechanism:** When edge irregularity exceeds 30% for 3 consecutive frames, a `STOP` command is dispatched over WebSockets to the ESP32, which commands the STM32 to halt motors.
- **Auto-Recovery:** If 3 consecutive good frames are verified after a stop, the conveyor automatically resumes operation.

---

## 📋 TODO & Hardware Validation Checklist

- [ ] Measure exact conveyor belt travel speed (cm/s) under load.
- [ ] Record ultrasonic response time and trigger distance margin.
- [ ] Document physical belt dimensions and roller gear ratio.
- [ ] Add hardware test bench photos and wiring harness schematics.

---

## 📄 License
This project is released under the [MIT License](LICENSE).
