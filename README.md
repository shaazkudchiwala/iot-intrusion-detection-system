# ESP32-Based IoT Intrusion Detection System

An academic IoT prototype that uses an ESP32 microcontroller with an HC-SR04 ultrasonic distance sensor to detect nearby objects and trigger local alerts and mobile notifications through ESP RainMaker cloud integration.

---

## Overview

This project demonstrates embedded IoT development using an ESP32, integrating hardware sensing, local alerting, and cloud-based notifications. The system monitors distance using an ultrasonic sensor and activates an audible alarm and visual indicators when an object approaches within a configurable threshold. Mobile alerts are sent through ESP RainMaker provisioning and notification infrastructure.

**Note:** This is an academic prototype system designed for educational purposes, not a production-grade security solution.

---

## Key Features

- **ESP32-based embedded controller** — Central processing unit for all system logic
- **HC-SR04 ultrasonic distance sensing** — Measures distance to nearby objects
- **Configurable intrusion threshold** — Detection distance set to 10 cm (user-configurable)
- **Green LED** — Indicates normal/clear state
- **Red LED** — Indicates intrusion state
- **Active buzzer** — Audible alarm on intrusion detection
- **Wi-Fi connectivity** — Enables cloud communication
- **ESP RainMaker cloud integration** — Provisioning and notification delivery
- **BLE-based provisioning** — Initial device setup via mobile app
- **Mobile intrusion notifications** — Push alerts to smartphone via RainMaker
- **Notification rate limiting** — Prevents alert spam during continuous intrusion
- **Serial Monitor output** — Real-time distance measurements and debug information
- **Automatic 30-second demonstration mode** — Built-in runtime limit for testing

---

## Hardware Components

| Component | Purpose |
|-----------|---------|
| **ESP32 Development Board** | Central microcontroller; processes sensor data and controls outputs |
| **HC-SR04 Ultrasonic Sensor** | Measures distance using time-of-flight ultrasonic pulses |
| **Green LED** | Visual indicator for normal/clear state |
| **Red LED** | Visual indicator for intrusion detection |
| **Active Buzzer** | Audible alarm triggered during intrusion |
| **Current-Limiting Resistors** | Protect LEDs from excessive current |
| **Voltage Divider** | Converts HC-SR04 ECHO signal from ~5 V to 3.3 V for ESP32 GPIO |
| **Breadboard** | Prototype circuit assembly |
| **Jumper Wires** | Circuit connections |
| **USB Cable** | Power and data connection to computer |
| **Smartphone** | Receives RainMaker mobile notifications |
| **Wi-Fi Network** | Enables ESP32 cloud connectivity |

---

## Pin Connections

| ESP32 Pin | Connected To | Purpose |
|-----------|--------------|---------|
| VIN | HC-SR04 VCC | Power supply to sensor |
| GND | Breadboard negative rail | Common ground reference |
| GPIO 5 | HC-SR04 TRIG | Trigger ultrasonic pulse |
| GPIO 18 | HC-SR04 ECHO (via voltage divider) | Receive echo pulse |
| GPIO 19 | Green LED (through resistor) | Normal state indicator |
| GPIO 21 | Red LED (through resistor) | Intrusion state indicator |
| GPIO 22 | Active Buzzer positive | Audible alarm output |
| GND | HC-SR04 GND, LED cathodes, Buzzer negative | Common ground for all components |

### ⚠️ Voltage Divider Requirement

The HC-SR04 ECHO signal must pass through a **voltage divider** before connecting to GPIO 18. The standard HC-SR04 can produce ECHO signals at or above 5 V, but ESP32 GPIO pins operate at 3.3 V logic levels. Connecting the ECHO signal directly to GPIO 18 risks damaging the microcontroller.

All components share a **common ground** connection.

---

## System Architecture

```
HC-SR04 Distance Sensor
        ↓
    ESP32 MCU
        ↓
Intrusion Decision Logic
    (10 cm threshold)
        ↓
   ┌────┴─────┐
   ↓          ↓
 Local      Wi-Fi
 Alerts    Output
 ├── Green LED    │
 ├── Red LED      │
 └── Buzzer       │
                  ↓
            ESP RainMaker
                Cloud
                  ↓
            Mobile Device
                  ↓
          Intrusion Alert
          Notification
```

The ESP32 serves as the central controller, reading the HC-SR04 sensor and making real-time intrusion decisions independently. Local outputs (LEDs, buzzer) operate without network dependency. Cloud notifications are sent only when Wi-Fi connectivity is available.

---

## How It Works

### Startup Sequence

1. ESP32 initializes and begins serial communication at 115200 baud
2. GPIO pins configured for HC-SR04 sensor and output devices
3. Green LED turned **ON**, Red LED and buzzer turned **OFF**
4. ESP RainMaker services initialized
5. BLE provisioning support activated for device setup
6. ESP32 attempts connection to previously provisioned Wi-Fi network
7. Intrusion monitoring loop begins

### Distance Measurement

The firmware continuously:

1. Sends a 10 microsecond trigger pulse on GPIO 5 (TRIG)
2. Measures the ECHO pulse duration on GPIO 18
3. Calculates distance using the time-of-flight formula: `distance = (duration × 0.0343) / 2`
4. Prints distance to Serial Monitor (115200 baud)
5. Waits 300 ms before next measurement

### Normal Condition (distance > 10 cm)

When the area is clear:
- **Green LED** → ON
- **Red LED** → OFF
- **Buzzer** → OFF
- **Notification state** → Reset (ready for next intrusion)

### Intrusion Condition (distance ≤ 10 cm)

When an object is detected within the threshold:
- **Green LED** → OFF
- **Red LED** → ON
- **Buzzer** → ON
- **Mobile alert** → Sent (if Wi-Fi connected)

---

## Intrusion Detection Logic

### Configuration Constants

```
Detection Threshold      = 10 cm
Notification Interval    = 10 seconds
Program Runtime Limit    = 30 seconds
```

### Notification Behavior

**First Detection:**
- Alert sent immediately when distance first becomes ≤ 10 cm

**Continuous Intrusion:**
- If the object remains within 10 cm for > 10 seconds, a second alert is sent
- Subsequent alerts follow the same 10-second interval

**Area Clear:**
- When distance exceeds 10 cm, the notification state resets
- A new intrusion event immediately triggers a fresh alert (no delay)

**Rate Limiting:**
- The 10-second interval prevents notification spam during extended intrusions
- Each alert includes the measured distance in the notification message

### Program Termination

After 30 seconds of runtime:
1. All outputs (LEDs and buzzer) are turned OFF
2. The intrusion monitoring loop terminates
3. The ESP32 enters an idle loop but remains powered
4. Serial output displays "30 SECOND TEST COMPLETE / SYSTEM STOPPING"

This 30-second limit is an intentional constraint for the academic demonstration. The ESP32 itself does not power down.

---

## ESP RainMaker Integration

### Purpose

ESP RainMaker provides:
- Initial device provisioning via BLE
- Wi-Fi and cloud connectivity
- Mobile push notification delivery
- Device management through the RainMaker mobile application

### Provisioning Process

```
ESP32 Boot
    ↓
BLE Advertising
    ↓
Scan with ESP RainMaker App
    ↓
BLE Connection
    ↓
Select Wi-Fi Network
    ↓
Enter Wi-Fi Password
    ↓
ESP32 Stores Credentials
    ↓
Connect to Wi-Fi
    ↓
Device Available in RainMaker App
    ↓
Ready to Receive Alerts
```

**Note:** BLE is used only for the initial provisioning process. After setup, all communication occurs over Wi-Fi.

### Alert Implementation

The firmware uses `esp_rmaker_raise_alert()` to send intrusion alerts. Each alert message includes:
- Intrusion confirmation text
- Measured distance (in centimeters)
- Timestamp (system time)

Alerts are delivered through the RainMaker notification system to registered mobile devices.

### Limitations

- RainMaker does **not** provide live distance telemetry in the mobile app
- Distance measurements are only sent during intrusion events (when notifications are triggered)
- Cloud delivery depends on Wi-Fi connectivity and RainMaker service availability
- Local detection and outputs operate independently of cloud connectivity

---

## Software Requirements

- **Arduino IDE** — Firmware compilation and upload tool
- **ESP32 Board Support** — Espressif Systems Arduino core library
- **ESP32 Arduino Core Version** — 3.3.10 (tested during development)
- **ESP RainMaker Libraries** — Included with ESP32 Arduino core
- **ESP RainMaker Mobile App** — For device provisioning and alert management
- **2.4 GHz Wi-Fi Network** — Device connectivity

### Arduino IDE Board Configuration

| Setting | Value |
|---------|-------|
| **Board** | ESP32 Dev Module |
| **Partition Scheme** | RainMaker 4MB No OTA |

**Important:** Avoid using "Erase All Flash Before Sketch Upload" unnecessarily. Flash erasing removes stored Wi-Fi credentials and RainMaker provisioning data. Use flash erase only when performing a complete device reset and reprovisioning is intended.

---

## Installation and Setup

### Step 1 — Install Arduino IDE

Download and install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software). This tool compiles and uploads firmware to the ESP32.

### Step 2 — Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File** → **Preferences**
3. Add this URL to "Additional Boards Manager URLs":
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to **Tools** → **Board** → **Boards Manager**
5. Search for "ESP32" and install the latest version by Espressif Systems

### Step 3 — Select the Board

1. In Arduino IDE, go to **Tools** → **Board** → **ESP32 Arduino** → Select **ESP32 Dev Module**

### Step 4 — Select the Partition Scheme

1. In Arduino IDE, go to **Tools** → **Partition Scheme** → Select **RainMaker 4MB No OTA**

This partition scheme reserves space for RainMaker provisioning data.

### Step 5 — Connect Hardware

Follow the [Pin Connections](#pin-connections) table to wire all components to the breadboard and ESP32 in the configuration specified.

### Step 6 — Connect ESP32 to Computer

Use a suitable USB data cable to connect the ESP32 development board to your computer. The USB connection provides power and communication for programming.

### Step 7 — Select COM Port

1. In Arduino IDE, go to **Tools** → **Port**
2. Select the COM port corresponding to the ESP32 (typically `COM3` or higher)
3. If the port is not visible, install the USB-to-serial driver for your ESP32 board

### Step 8 — Upload Firmware

1. Open the `IntrusionDetectionCode.ino` sketch in Arduino IDE
2. Click the **Upload** button (→ icon) or press **Ctrl+U**
3. Wait for compilation and upload to complete
4. Serial Monitor will show initialization messages once upload is successful

### Step 9 — Provision the Device with RainMaker

1. Power on the ESP32
2. Open the **ESP RainMaker** mobile app on your smartphone
3. Tap **+ Add Device**
4. Scan the QR code displayed in the Serial Monitor during provisioning
5. Select your 2.4 GHz Wi-Fi network and enter the password
6. Follow the app prompts to complete provisioning
7. Device will appear in the RainMaker app when Wi-Fi connection succeeds

**Do not expose the RainMaker Proof-of-Possession (PoP) value or Wi-Fi credentials in public repositories.**

---

## Configuration

Key constants in the firmware can be modified to adjust system behavior:

### Detection Threshold
```cpp
const float INTRUDER_DISTANCE = 10.0;  // cm
```
Adjusts the distance threshold for intrusion detection. Lower values increase sensitivity.

### Program Runtime
```cpp
const unsigned long PROGRAM_TIME = 30000;  // milliseconds
```
Limits the total runtime of the intrusion monitoring routine. After this time, outputs are turned OFF and the monitoring loop terminates.

### Alert Interval
```cpp
const unsigned long ALERT_INTERVAL = 10000;  // milliseconds
```
Minimum time between repeated notifications during a continuous intrusion. Lower values increase alert frequency.

Modify these values in the source code and re-upload to apply changes.

---

## Runtime Behavior

| Condition | Green LED | Red LED | Buzzer | Mobile Alert | Notes |
|-----------|-----------|---------|--------|--------------|-------|
| Distance > 10 cm | ON | OFF | OFF | None | Normal/clear state |
| Distance ≤ 10 cm (first) | OFF | ON | ON | Immediate | Intrusion detected |
| Intrusion persists > 10 sec | OFF | ON | ON | Sent | Repeated alert |
| Object leaves detection zone | ON | OFF | OFF | None | State resets |
| 30 seconds elapsed | OFF | OFF | OFF | None | Monitoring stops |

**Alert Behavior:** Alerts include the current measured distance. For example: `"INTRUDER DETECTED! Distance: 8.5 cm"`

---

## Serial Monitor

The firmware communicates via serial at **115200 baud**. Connect the Serial Monitor to view real-time distance measurements and system events.

### Example Output

```
========================================
   ESP32 INTRUSION DETECTION SYSTEM
========================================
Hardware initialized.

RainMaker provisioning started.
RainMaker initialized.
Intrusion monitoring started.
----------------------------------------
Distance: 25.4 cm
Area clear.

Distance: 23.1 cm
Area clear.

Distance: 8.2 cm
!!! INTRUDER DETECTED !!!

Sending RainMaker notification...
RainMaker notification sent.

Distance: 7.9 cm
!!! INTRUDER DETECTED !!!

Distance: 8.5 cm
!!! INTRUDER DETECTED !!!

Sending RainMaker notification...
RainMaker notification sent.

Distance: 15.3 cm
Area clear.

Distance: 28.0 cm
Area clear.

========================================
       30 SECOND TEST COMPLETE
       SYSTEM STOPPING
========================================
```

**Note:** Actual distance values will vary based on physical setup and object reflectivity.

---

## Testing

| Test Case | Expected Result |
|-----------|-----------------|
| No object within 10 cm | Green LED ON, Red LED OFF, Buzzer OFF, no alert |
| Object detected within 10 cm | Green LED OFF, Red LED ON, Buzzer ON, mobile alert sent |
| First intrusion | Immediate RainMaker notification |
| Intrusion persists 10+ seconds | Additional notification sent |
| Intrusion persists 20+ seconds | Another notification sent (continuing every 10 sec) |
| Object leaves detection zone | Green LED ON, Red LED OFF, Buzzer OFF, notification state reset |
| Wi-Fi unavailable | Local detection operates normally; cloud alerts cannot be sent |
| 30 seconds elapsed | Outputs turn OFF and monitoring routine terminates |

**Note:** Actual mobile notification timing may vary slightly due to network latency and cloud processing delays.

---

## Electrical and Safety Notes

- **GPIO voltage levels:** ESP32 GPIO pins operate at 3.3 V logic. Ensure all input signals respect this limit.
- **Voltage divider requirement:** The HC-SR04 ECHO signal must use a voltage divider to convert from 5 V to 3.3 V before reaching GPIO 18.
- **LED protection:** All LEDs must have current-limiting resistors to prevent excessive current and damage.
- **Common ground:** All components (sensor, LEDs, buzzer) must share a common ground connection to the ESP32.
- **GPIO current limits:** ESP32 GPIO pins have limited current sourcing capability (~40 mA typical). Do not exceed GPIO specifications.
- **Active buzzer:** An active buzzer includes internal oscillation circuitry and requires only power/ground connection. Passive buzzers require PWM signal.
- **Higher current loads:** For loads exceeding GPIO current limits, use an external transistor or MOSFET driver circuit.
- **Power supply:** Use a stable 5 V power supply for the HC-SR04 and other 5 V components. Power can be drawn from the ESP32's VIN pin or an external source.

---

## Limitations

1. **Object detection only** — The HC-SR04 detects the presence and distance of nearby objects; it does not identify whether the object is a person, animal, or inanimate object.
2. **No human recognition** — This system cannot distinguish between a person, a chair, a bag, or any other object within the detection zone.
3. **Environmental sensitivity** — Detection can be affected by object shape, surface material, angle of reflection, ambient temperature, humidity, and background noise.
4. **No AI/ML employed** — The system uses simple distance-based logic; no artificial intelligence or machine learning is involved.
5. **Network dependency for cloud alerts** — Mobile notifications require active Wi-Fi connectivity and RainMaker service availability.
6. **No live telemetry** — The RainMaker app does not display real-time distance values; only alert notifications are sent.
7. **Academic prototype** — This system is designed for educational purposes and learning, not for production security applications.
8. **Limited range** — HC-SR04 range is typically 2–400 cm, depending on reflectivity.
9. **Blind zone** — Objects very close to the sensor (within ~2 cm) may not be detected reliably.
10. **30-second demo limit** — The firmware intentionally stops monitoring after 30 seconds; this is a built-in demonstration constraint.

---

## Project Structure

```
iot-intrusion-detection-system/
├── README.md                              # This file
├── LICENSE                                # Project license
├── src/
│   └── IntrusionDetectionCode.ino         # Main intrusion detection firmware
├── RainMakerNotificationTest/
│   └── RainMakerNotificationTest.ino      # Standalone RainMaker notification test
└── media/
    └── Video-Link.txt                     # Reference to project video
```

### RainMakerNotificationTest

The `RainMakerNotificationTest.ino` is a standalone utility sketch used to verify RainMaker provisioning and notification delivery independently from the main intrusion detection system. This test:

1. Initializes RainMaker provisioning
2. Waits for Wi-Fi connection
3. Sends a single test notification after 5 seconds
4. Completes and stops after 20 seconds

This utility was used during development to confirm that the RainMaker notification system was functioning correctly before integrating it into the full intrusion detection firmware. It is not part of the main system deployment.



---

## Technologies Used

**Hardware:**
- ESP32 microcontroller
- HC-SR04 ultrasonic sensor
- LEDs and active buzzer
- Resistors and breadboard

**Software & Frameworks:**
- C++ / Arduino language
- Arduino IDE
- ESP32 Arduino Core (version 3.3.10)
- ESP RainMaker platform
- Wi-Fi connectivity
- BLE provisioning

---

## Development Approach

The project was developed incrementally:

1. **ESP32 connectivity** — Tested basic board operation and USB communication
2. **Hardware integration** — Verified GPIO control for LEDs and buzzer, HC-SR04 sensor readings
3. **RainMaker provisioning** — Tested device setup and BLE provisioning flow independently
4. **Mobile notifications** — Verified alert delivery through RainMaker app
5. **System integration** — Combined all components into the final intrusion detection firmware

The current implementation represents the final integration of all tested components.

---

## Future Improvements

Potential extensions to this academic project (not currently implemented):

- **Multi-sensor deployment** — Array of ultrasonic sensors for wider coverage
- **PIR sensor integration** — Passive infrared motion detection as supplementary trigger
- **Temperature/humidity monitoring** — Log environmental conditions with alerts
- **Local web dashboard** — Real-time monitoring through a local web interface
- **MQTT integration** — Standard IoT messaging protocol for broader platform compatibility
- **Event logging** — Persistent storage of intrusion events with timestamps
- **Battery backup** — Uninterruptible operation during power outages
- **OTA firmware updates** — Over-the-air update capability
- **Multiple notification channels** — SMS, email, or Telegram in addition to RainMaker
- **Geofencing** — Integration with smartphone location for context-aware alerting
- **Tamper detection** — Hardware sensors to detect device tampering
- **Advanced telemetry** — Expose distance history or statistics through RainMaker

---

## Author

**Shaaz Kudchiwala**  
B.Tech Robotics & AI Engineering

---

## License

This project is licensed under the terms specified in the LICENSE file.

---

## References

- [ESP32 Official Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Arduino IDE Guide](https://docs.arduino.cc/software/ide-v2/)
- [HC-SR04 Ultrasonic Sensor Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Ultrasonic/HCSR04.pdf)
- [ESP RainMaker Documentation](https://rainmaker.espressif.com/)
