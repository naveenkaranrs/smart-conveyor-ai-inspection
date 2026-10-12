# Smart Conveyor AI Inspection System 📦🔍

**Industrial Automation | Embedded Vision | IoT | STM32 | ESP32**

A real-time smart conveyor inspection system that combines embedded control, sensor fusion, IoT connectivity, and vision-based monitoring to automate package inspection in industrial environments.

The system utilizes an **STM32 microcontroller** for conveyor motor control and sensor processing, while an **ESP32 gateway** provides wireless communication, remote monitoring, and a web-based dashboard. An integrated camera stream enables visual inspection, creating a low-cost and scalable Industry 4.0 solution.

---

## Project Overview

Quality inspection remains a critical challenge in manufacturing and packaging facilities. Traditional inspection methods rely heavily on human operators, resulting in:

- Human fatigue
- Missed defects
- Inconsistent inspection quality
- Lack of real-time monitoring
- Limited data logging

This project addresses those issues by implementing an IoT-enabled conveyor inspection platform that automatically detects packages, pauses the conveyor at inspection points, streams live video, and provides remote monitoring through a web dashboard.

---

## Key Features

### Automated Package Detection

- IR sensor-based object detection
- Automatic conveyor stopping at inspection zone

### Sensor Fusion

- IR Sensor for package presence detection
- HC-SR04 ultrasonic sensor for object positioning
- Combined sensing improves inspection reliability

### Embedded Motor Control

- STM32-based conveyor control
- PWM speed regulation
- Automatic and manual operating modes

### Real-Time Dashboard

- Hosted on ESP32
- Wi-Fi enabled monitoring
- Live status updates using WebSockets

### Vision-Based Inspection

- Mobile camera MJPEG streaming
- Real-time visual inspection
- Future-ready AI integration

### Remote Monitoring

- Start/Stop conveyor remotely
- Speed control through browser
- View system logs in real time

---

# System Architecture

```text
 ┌─────────────────────┐
 │     Package         │
 └──────────┬──────────┘
            │
            ▼

 ┌─────────────────────┐
 │     IR Sensor       │
 │ Object Detection    │
 └──────────┬──────────┘
            │
            ▼

 ┌─────────────────────┐
 │      STM32          │
 │ Motor Control Unit  │
 └─────┬─────────┬─────┘
       │         │
       │         ▼
       │   HC-SR04 Sensor
       │  Distance Measurement
       │
       ▼

 ┌─────────────────────┐
 │    UART Link        │
 └──────────┬──────────┘
            │
            ▼

 ┌─────────────────────┐
 │      ESP32          │
 │ IoT Gateway         │
 └──────────┬──────────┘
            │
            ▼

 ┌─────────────────────┐
 │ Web Dashboard       │
 │ WebSocket Updates   │
 └──────────┬──────────┘
            │
            ▼

 ┌─────────────────────┐
 │ Camera Stream       │
 │ Live Inspection     │
 └─────────────────────┘
```

---

# Hardware Components

| Component | Function |
|------------|----------|
| STM32 Nucleo | Main Controller |
| ESP32 | IoT Communication Gateway |
| L298N Driver | Motor Driver |
| DC Motor | Conveyor Drive |
| IR Sensor | Package Detection |
| HC-SR04 | Distance Measurement |
| Mobile Camera | Live Inspection Stream |
| Power Supply | System Power |

---

# Pin Configuration

## STM32

| Device | Pin |
|----------|------|
| IR Sensor OUT | PA0 |
| HC-SR04 TRIG | PB13 |
| HC-SR04 ECHO | PB14 |
| L298N IN1 | PA1 |
| L298N IN2 | PA2 |
| L298N ENA | PA8 |

---

## ESP32

| Function | GPIO |
|-----------|--------|
| RX | GPIO16 |
| TX | GPIO17 |

### UART Communication

```text
STM32 TX ───► ESP32 RX

STM32 RX ◄─── ESP32 TX

Common GND
```

---

# Software Architecture

```text
+----------------------------------+
|  Industrial Web Dashboard        |
+----------------------------------+
|  ESP32 Communication Layer       |
|  WebSocket Server                |
|  HTTP Server                     |
+----------------------------------+
|  UART Communication              |
+----------------------------------+
|  STM32 Control Layer             |
|  Sensor Processing               |
|  Motor Control                   |
+----------------------------------+
|  Hardware Layer                  |
+----------------------------------+
```

---

# System Workflow

```text
Start Conveyor
       │
       ▼

Package Detected
(IR Sensor)
       │
       ▼

STM32 Stops Conveyor
       │
       ▼

Distance Validation
(HC-SR04)
       │
       ▼

Camera Inspection
       │
       ▼

Dashboard Update
(WebSocket)
       │
       ▼

Resume Conveyor
       │
       ▼

Next Package
```

---

# Communication Protocol

## STM32 → ESP32

Examples:

```text
IR:1 DIST:15

STATUS:RUNNING

STATUS:STOPPED

EVENT:PACKAGE_DETECTED

SPEED:150
```

---

## ESP32 → STM32

Commands:

```text
START

STOP

S150

AUTOON

AUTOOFF
```

---

# Industrial Dashboard Features

The ESP32 hosts a responsive browser-based interface that provides:

### Real-Time Monitoring

- Conveyor status
- Package detection status
- Ultrasonic distance readings
- Speed percentage

### Camera Monitoring

- Live MJPEG video stream
- Top-view inspection
- Side-view inspection

### Control Functions

```text
START CONVEYOR

STOP CONVEYOR

Speed Adjustment

Automatic Mode

Manual Mode
```

### Inspection Logs

```text
[14:20:02]
Package Detected

[14:20:04]
Conveyor Stopped

[14:20:07]
Inspection Completed

[14:20:10]
Conveyor Running
```

---

# Test Scenarios

## Test 1: Normal Operation

Expected Result:

```text
Package detected

Conveyor stopped

Inspection performed

Conveyor resumed
```

Status:

✅ Passed

---

## Test 2: No Package Condition

Expected Result:

```text
Continuous conveyor operation
```

Status:

✅ Passed

---

## Test 3: Distance Measurement

Expected Result:

```text
Correct package positioning
```

Status:

✅ Passed

---

## Test 4: Dashboard Monitoring

Expected Result:

```text
Real-time updates with zero refresh
```

Status:

✅ Passed

---

# Engineering Concepts Demonstrated

- Embedded Systems Design
- Industrial Automation
- Sensor Fusion
- UART Communication
- PWM Motor Control
- IoT Monitoring
- WebSocket Communication
- Real-Time Dashboards
- Embedded Networking
- Industry 4.0 Concepts
- Vision-Based Inspection
- Human-Machine Interfaces (HMI)

---

# Future Enhancements

## AI-Based Defect Detection

Integrate:

- YOLO
- TensorFlow Lite
- OpenCV

for automatic defect classification.

### Cloud Integration

- AWS IoT
- Firebase
- Azure IoT Hub

### Edge AI

Run defect detection directly on:

- ESP32-S3
- Raspberry Pi
- NVIDIA Jetson Nano

### Advanced Analytics

- Production reports
- Quality statistics
- Defect trend analysis

### Industrial Deployment

- PLC integration
- SCADA compatibility
- Ethernet/IP support

---

# Repository Structure

```text
smart-conveyor-ai-inspection/

├── firmware/
│   ├── stm32/
│   │   ├── conveyor_controller.ino
│   │   └── sensors.c
│   │
│   └── esp32/
│       ├── dashboard_server.ino
│       └── websocket_handler.cpp
│
├── dashboard/
│   ├── index.html
│   ├── styles.css
│   └── script.js
│
├── docs/
│   ├── architecture.png
│   ├── workflow.png
│   └── report.pdf
│
├── images/
│
├── README.md
│
└── LICENSE
```

---

# Results

The system successfully demonstrated:

✅ Real-time package detection

✅ Automated conveyor control

✅ Reliable UART communication

✅ Live dashboard monitoring

✅ Wireless access through ESP32

✅ Real-time camera streaming

✅ Low-cost industrial inspection architecture

---

# Author

**Naveen Karan R. S.**

Electronics and Communication Engineering  
Embedded Systems | IoT | Industrial Automation | Edge AI

GitHub: **[@naveenkaranrs](https://github.com/naveenkaranrs)**

---

# License

Distributed under the MIT License.

See the `LICENSE` file for more information.

> Building Industry 4.0 solutions through embedded intelligence, industrial automation, and real-time connected systems.
