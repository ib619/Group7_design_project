# Y2 Group 7 EEE/EIE Design Project Repository

<br />

## TABLE OF CONTENTS

- [1. TEAM MEMBERS](#1-team-members)
- [2. DEPENDENCIES](#2-dependencies)
  - [2.1. List of non-standard Arduino libraries needed for final rover](#21-list-of-non-standard-arduino-libraries-needed-for-final-rover)
  - [2.2. Tools needed to configure final rover](#22-tools-needed-to-configure-final-rover)
- [3. ARDUINO IDE SETTINGS](#3-arduino-ide-settings)
  - [3.1. ESP32 Upload Settings](#31-esp32-upload-settings)
  - [3.2. Arduino Upload Settings](#32-arduino-upload-settings)
  - [3.3. Known Problems With Uploading Scripts](#33-known-problems-with-uploading-scripts)
- [4. ROVER](#4-rover)
  - [4.1. First Time Setup](#41-first-time-setup)
  - [4.2. Rover Setup](#42-rover-setup)
  - [4.3. Sending Instructions to Rover](#43-sending-instructions-to-rover)

<br />

## 1. TEAM MEMBERS
- Command - Zhao, Siting (01709075)
- Control - Lim, Joshua (01578189)
- Drive - Bodnar, Igor (01739540)
- Energy - Chau, Yan To (01705211)
- Integration - Kitikul, Chackrarat (01743965)
- Vision - Tan, Si Yu (01701371)

<br />

## 2. DEPENDENCIES

### 2.1. List of non-standard Arduino libraries needed for final rover:
1. [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
2. [ESP32 API](https://github.com/espressif/arduino-esp32) 
3. INA219_WE (Download from Arduino library manager)
4. PubSubClient (Download from Arduino library manager)
5. Arduino MegaAVR Boards (Download from Arduino library manager)

### 2.2. Tools needed to configure final rover:
1. [Arduino IDE 1.8.15](https://www.arduino.cc/en/software)
2. [npm](https://www.npmjs.com/get-npm)
3. [Python 3.X.X](https://www.python.org/downloads/)
4. [Mosquitto MQTT Broker v2.0.10](https://mosquitto.org/download/)
5. [Quartus 16.1](https://fpgasoftware.intel.com/16.1/)
6. NIOS II (comes with Quartus 16.1 installation)

<br />

## 3. ARDUINO IDE SETTINGS

### 3.1. ESP32 Upload Settings:
- Board Name/Type: ESP32 Dev Module
- Partition: HUGE APP

### 3.2. Arduino Upload Settings:
- Board Name/Type: Arduino Nano Every

### 3.3. Known Problems With Uploading Scripts
- Arduino will not accept sketch uploads if SMPS is not powered
- ESP32 will not upload if UART cables are still plugged in

<br />

## 4. ROVER

### First Time Setup
- Follow "How to..." instructions within the Command_Subsystem folder for either local or AWS server
- Follow "Drive_Control_integration.ino" instructions within the Drive_Subsystem folder to setup APIs for the Arduino
- Configure `config.h` in `/path/Control_Subsystem/Control_program/control_program` using the instructions within Control_Subsystem folder

### 4.2. Rover Setup
1. Turn on SMPS power, unplug UART cables
2. Follow "How to start..." instructions within Command_Subsystem folder for either local or AWS server
3. Run 'start_server.sh' within `/path/Command_Subsystem/server`
4. Upload the .sof file found in `/path/Vision_Subsystem/sof_elf_files/v8_final` using the Quartus Programmer
5. Upload the .elf file found in `/path/Vision_Subsystem/sof_elf_files/v8_final` using nios2
6. Upload the Arduino file `Drive_Control_Integration.ino` found in `/path/Drive_Subsystem/Arduino_Code` using the Arduino IDE with the correct settings
7. Unplug the UART cables from the ESP32
8. Upload the ESP32 file found in `/path/Control_Subsystem/Control_program/control_program` using the Arduino IDE with the correct settings
9. Plug in the UART cables
10. Wait for LED8 on SMPS to turn on before sending instructions

### 4.3. Sending Instructions to Rover
1. Navigate to the website `npm start` opened
2. Login using the appropriate credentials found within the Command_Subsystem folder
3. Select Discrete or Coordinate mode from the top menu
4. Send instructions using boxes the provided fields
