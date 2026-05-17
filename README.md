# Multi-Zone Smart Security System

![Platform](https://img.shields.io/badge/Platform-Arduino%20Mega%202560-00979D?logo=arduino&logoColor=white)
![IDE](https://img.shields.io/badge/IDE-PlatformIO-FF7F00?logo=platformio&logoColor=white)
![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-00599C?logo=c&logoColor=white)
![Simulation](https://img.shields.io/badge/Simulation-Wokwi-7B68EE?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCI+PHBhdGggZmlsbD0id2hpdGUiIGQ9Ik0xMiAyQzYuNDggMiAyIDYuNDggMiAxMnM0LjQ4IDEwIDEwIDEwIDEwLTQuNDggMTAtMTBTMTcuNTIgMiAxMiAyem0tMSAxNEg5VjhIMTF2OHptNCAwaC0yVjhoMnY4eiIvPjwvc3ZnPg==)
![License](https://img.shields.io/badge/License-MIT-green)

An Arduino-based security system designed to monitor multiple independent zones in real time. Each zone is equipped with its own sensors, alarm output, and access credentials, giving granular control over each monitored area. The system features a local control board, two-step authentication, persistent credential storage, and Wi-Fi connectivity for remote event alerts.

---

## Table of Contents

- [Overview](#overview)
- [Demo](#demo)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Circuit & Simulation](#circuit--simulation)
- [Wi-Fi Remote Alerts](#wi-fi-remote-alerts)
- [Software & Dependencies](#software--dependencies)
- [File Structure](#file-structure)
- [Known Limitations](#known-limitations)
- [Future Work](#future-work)
- [License](#license)

---

## Overview

This project was developed as part of an electrical engineering curriculum using the **Arduino Mega 2560** microcontroller. The system divides a monitored area into **three independently controlled zones**. Each zone can be armed or disarmed individually using its own credentials, or all zones can be managed at once using a master credential set.

A central control board serves as the user interface, built around an LCD screen, a 4×4 keypad, and an RFID reader. Access control is enforced through **two-step authentication**: the user must first enter a password, then scan a registered RFID tag. This dual-factor approach means that knowing the password alone is not enough to arm or disarm a zone.

When a breach is detected — whether by motion or by a door/window being opened — the system activates the zone's alarm output, displays the breach details on the LCD screen, and sends a notification through the Wi-Fi module to an **online alert portal**. The notification identifies which zone was breached and which specific sensor triggered it.

All credentials (passwords and RFID tag IDs) are stored in the Arduino's **EEPROM**, meaning they persist across power cycles without needing to be re-entered.

---

## Demo

> 🎥 Click on the image to start the video...

<a href="https://youtu.be/MN1zN7_iqPo">
  <img src="https://github.com/user-attachments/assets/434a98ae-35c6-4c1a-b8e9-30de9684ebd4" width="800" alt="Watch the video">
</a>

---

## Features

- **Three independently monitored zones**, each with dedicated sensors, alarm output, and credentials
- **Two-step authentication** — password entry followed by RFID tag scan — required to arm or disarm any zone
- **Master credentials** for system-wide arm/disarm with a single authentication
- **Per-zone sensing** — one PIR motion sensor and two magnetic reed switches (door and window) per zone
- **Real-time alarm display** on the LCD, identifying the breached zone and the specific triggered sensor; when multiple zones are breached simultaneously, the display cycles through each alert automatically
- **Wi-Fi connectivity** via ESP8266 module for remote event logging and instant online notifications
- **Persistent credential storage** in EEPROM — all passwords and RFID tags survive power loss
- **In-system credential management** — passwords and RFID tags can be updated directly from the control board without reprogramming
- **Non-blocking architecture** — the system polls sensors and manages timers without halting the main loop

> **Note:** In the Wokwi simulation, zone buzzers are replaced with LEDs for easier visual feedback during testing.

---

## Hardware Requirements

| Component | Quantity |
|---|---|
| Arduino Mega 2560 | 1 |
| ESP8266 Wi-Fi Module | 1 |
| MFRC522 RFID Reader | 1 |
| RFID Tags / Cards | 1 per credential set (4 total in default config) |
| 16×2 LCD Screen | 1 |
| 4×4 Matrix Keypad | 1 |
| PIR Motion Sensor | 3 (one per zone) |
| Magnetic Reed Switch | 6 (two per zone: door + window) |
| Buzzer | 3 (one per zone) |
| Breadboard | 1 |
| Breadboard Power Supply (5 V / 3.3 V) | 1 |
| Jumper Wires | As needed |

> Component quantities for sensors, alarms, and credentials scale with the number of monitored zones. The values above reflect the default 3-zone configuration.

---

## Circuit & Simulation

### Circuit Diagram

The schematic below shows the full wiring of all components to the Arduino Mega 2560. Key communication interfaces used are:

- **SPI** — RFID reader (MFRC522), shared with any SPI peripherals via separate chip-select pins
- **Serial1 (pins 18/19)** — ESP8266 Wi-Fi module
- **4-bit parallel** — LCD screen
- **Digital I/O** — PIR sensors, reed switches, and alarm outputs

<img width="1139" height="901" alt="Circuit_diagram" src="https://github.com/user-attachments/assets/c5e53bb4-414a-422a-a5ae-aa53b8bde9e8" />


---

### Online Simulation (Wokwi)

The full circuit is available as a **live interactive simulation on Wokwi**. It runs entirely in the browser — no hardware, software installation, or account required. The simulation reflects the exact same firmware that runs on physical hardware.

🔗 **[Open the Wokwi Simulation](https://wokwi.com/projects/464290971084275713)**

For step-by-step instructions on how to operate the simulation — including how to enter passwords, scan RFID tags, trigger sensors, and run test scenarios — refer to the dedicated simulation guide included in the Wokwi project.

---

## Wi-Fi Remote Alerts

The system uses an **ESP8266 Wi-Fi module** connected to the Arduino Mega via the Serial1 hardware UART (pins 18 TX / 19 RX). When a relevant security event occurs, the module sends an HTTP request to the **[IFTTT Webhooks](https://ifttt.com/maker_webhooks)** service, which can forward the notification to email, a phone notification, a spreadsheet log, or any other IFTTT-supported action.

### Events That Trigger a Notification

| Event | Example Message Sent |
|---|---|
| Zone breached | `"SECURITY ALERT AT"` + zone name + sensor name |
| System armed or disarmed | `"System is armed"` / `"System disarmed"` |
| Invalid password entered | `"Incorrect password was entered"` |
| Invalid RFID tag scanned | `"Invalid tag was scanned"` |
| RFID scan timed out | `"No tag scanned"` |

### Connection Behaviour

- On startup, the system attempts to connect to the configured Wi-Fi network. A success or failure message is printed to the Serial monitor.
- If the connection is lost at runtime, the system automatically retries every **30 seconds** without interrupting normal security operation.
- If Wi-Fi is unavailable, all local alarm functions (LCD display, buzzer/LED) continue to operate normally. Only the remote notification is skipped.

### How to Configure

All Wi-Fi and IFTTT settings are defined at the top of `WiFiHandler.cpp`. Fill in these four values before flashing:

```cpp
const char* WIFI_SSID       = "Your_Network_Name";
const char* WIFI_PASS       = "Your_Network_Password";
const char* IFTTT_EVENT_NAME = "security_breach";   // The event name you created on IFTTT
const char* IFTTT_KEY       = "your_ifttt_key";     // From ifttt.com/maker_webhooks/settings
```

To obtain your IFTTT key and create a Webhooks trigger:
1. Go to [ifttt.com](https://ifttt.com) and sign in.
2. Search for **Webhooks** and open the service.
3. Click **Documentation** to find your personal API key.
4. Create a new Applet with **Webhooks** as the trigger and any action you prefer (email, push notification, etc.).
5. Use the same event name in both the Applet and in `IFTTT_EVENT_NAME` above.

> **Note:** Wi-Fi functionality is not available in the Wokwi simulation. In the simulation, the `WiFiHandler` module is excluded from `main.cpp`, and all alerts are local only (LCD + LED).

---

## Software & Dependencies

The firmware is written in **C/C++** and built using **[PlatformIO](https://platformio.org/)**, an open-source embedded development ecosystem. PlatformIO handles dependency management, compilation, and flashing automatically via the `platformio.ini` configuration file.

### Libraries

| Library | Purpose |
|---|---|
| `arduino-libraries/LiquidCrystal` | LCD display control |
| `chris--a/Keypad` `^3.1.1` | 4×4 matrix keypad input handling |
| `jandrassy/WiFiEspAT` `^2.0.0` | Wi-Fi communication via ESP8266 |
| `miguelbalboa/MFRC522` `^1.4.12` | RFID reader interfacing |
| `adafruit/Adafruit BusIO` `^1.17.4` | I2C/SPI abstraction layer (Adafruit dependency) |

All libraries are declared in `platformio.ini` and installed automatically when the project is built for the first time.

---

## File Structure

```
├── src/
│   ├── main.cpp                  # Main entry point; state machine and module orchestration
│   ├── Config.h                  # Pin assignments, timing constants, and system-wide settings
│   ├── ConfigDataHandler.h/.cpp  # EEPROM read/write for secure credential storage
│   ├── KeypadHandler.h/.cpp      # Keypad scanning and password input processing
│   ├── LcdHandler.h/.cpp         # LCD output: status messages, prompts, and breach alerts
│   ├── RfidHandler.h/.cpp        # RFID reader communication and UID verification
│   ├── SensorHandler.h/.cpp      # PIR and reed switch monitoring across all zones
│   └── WiFiHandler.h/.cpp        # Wi-Fi connection management and IFTTT event reporting
├── diagram.png                   # Circuit diagram 
├── Demo.mp4                      # Project showcase
└── platformio.ini                # PlatformIO project configuration and library list
```

### Module Descriptions

- **`main.cpp`** — Initializes all subsystems and runs the main control loop as a finite state machine with five states: `DISARMED`, `ARMED`, `TRIGGERED`, `CONFIG_MODE`, and `ERROR_MODE`.
- **`Config.h`** — Centralizes all pin assignments, EEPROM address layout, timing constants (e.g. RFID scan timeout, sensor debounce delay), and zone configuration.
- **`ConfigDataHandler`** — Manages reading from and writing to EEPROM for all zone passwords and RFID tag UIDs, including the master credential set. On first boot, factory default credentials are automatically written.
- **`KeypadHandler`** — Handles keypad scanning and translates raw input into password strings, including backspace and submission detection.
- **`LcdHandler`** — Abstracts all LCD output with two display modes: a temporary message (shown for a fixed duration) and a persistent base message (shown until replaced). This ensures transient alerts are never cut short by background screen updates.
- **`RfidHandler`** — Manages communication with the MFRC522 reader, reads tag UIDs, and formats them for comparison against stored credentials.
- **`SensorHandler`** — Polls PIR sensors and reed switches across all three zones and reports breach events, including which zone and which specific sensor was triggered.
- **`WiFiHandler`** — Establishes the ESP8266 Wi-Fi connection on startup, maintains it with automatic retry logic, and sends structured HTTP GET requests to the IFTTT Webhooks endpoint on security events.

---

## Known Limitations

- **No encrypted credential storage.** Passwords and RFID tag UIDs are stored as plain text in the Arduino's EEPROM. Anyone with physical access to the board and an EEPROM reader could extract the credentials directly.

- **Single RFID tag per zone.** Each zone supports exactly one registered RFID tag. There is no support for multiple authorised users per zone, and no way to tell from the system logs which individual performed an action.

- **No event timestamps.** The system has no real-time clock (RTC) module. Breach and access events are reported without a date or time, making it difficult to reconstruct a timeline from logs.

- **Notifications sent over HTTP.** Communication with the IFTTT Webhooks service uses plain HTTP, not HTTPS. On an untrusted network, notification content could theoretically be intercepted.

- **Wi-Fi connectivity is not fault-tolerant.** If the network is unavailable for an extended period, there is no local queue of missed events. Notifications that fail to send are silently dropped.

- **Zone count is hardcoded.** The number of zones (`NUM_ZONES = 3`) is a compile-time constant. Changing it requires code modifications and re-flashing, and the hardware must be wired accordingly.

- **Password constraints are minimal.** The system enforces a minimum length of 4 characters and a maximum of 8 digits, but imposes no complexity requirements. Simple numeric passwords like `1111` are accepted.

- **EEPROM write endurance.** The Arduino's EEPROM is rated for approximately 100,000 write cycles per address. Frequent credential changes over the lifetime of the device will eventually degrade storage reliability.

- **No tamper detection.** The system does not detect physical interference with the sensors, wiring, or the control board itself (e.g. a sensor being disconnected or covered).

---

## Future Work

- **Encrypted credential storage.** Passwords and RFID UIDs could be hashed or encrypted before being written to EEPROM, making it significantly harder to extract usable credentials from the hardware.

- **Multi-user access management.** Support for multiple RFID tags per zone would allow different individuals to be authorised independently, each with their own tag, and would enable per-user access logging.

- **Event logging with timestamps.** Integrating an RTC module (e.g. DS3231) and an SD card would allow the system to maintain a local, time-stamped log of all access and breach events — useful for auditing and incident review.

- **Secure remote notifications (HTTPS).** Replacing the current HTTP-based IFTTT integration with an HTTPS connection, or migrating to a dedicated backend API, would protect notification data in transit.

- **Dedicated mobile application.** A companion mobile app could replace the IFTTT dependency, offering a custom notification interface, a live system status dashboard, and remote arm/disarm capability.

- **Scalable zone architecture.** Refactoring the zone configuration to be fully data-driven at runtime (rather than a compile-time constant) would allow the number of monitored zones to be adjusted without modifying or re-flashing the firmware.

- **Battery backup.** Adding a small UPS or battery module would keep the system operational during power outages — a critical gap for any real security application.

- **Tamper detection.** Monitoring for unexpected sensor disconnections or abnormal signal patterns could allow the system to raise an alert if someone attempts to physically defeat a sensor.

- **Over-the-air (OTA) firmware updates.** Leveraging the ESP8266 module to support OTA updates would allow firmware improvements to be deployed without physical access to the board.

- **Web-based configuration interface.** Rather than navigating the keypad menu to update credentials, a small web interface served by the ESP8266 could provide a more convenient way to manage settings remotely.

---

## License

This project is licensed under the [MIT License](LICENSE).
