# Y2 Group 7 EEE/EIE Design Project Repository

<br />

## DEPENDENCIES

### List of non-standard Arduino libraries needed for final rover:
1. [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
2. [ESP32 API](https://github.com/espressif/arduino-esp32) 
3. INA219_WE (Download from Arduino library manager)
4. PubSubClient (Download from Arduino library manager)
5. Arduino MegaAVR Boards (Download from Arduino library manager)

### Tools needed to configure final rover:
1. [Arduino IDE 1.8.15](https://www.arduino.cc/en/software)
2. [npm](https://www.npmjs.com/get-npm)
3. [Python 3.X.X](https://www.python.org/downloads/)
4. [Mosquitto MQTT Broker v2.0.10](https://mosquitto.org/download/)
5. [Quartus 16.1](https://fpgasoftware.intel.com/16.1/)
6. NIOS II (comes with Quartus 16.1 installation)

<br />

## ARDUINO IDE SETTINGS

### ESP32 Upload Settings:
- Board Name/Type: ESP32 Dev Module
- Partition: HUGE APP

### Arduino Upload Settings:
- Board Name/Type: Arduino Nano Every

### Known Problems With Uploading Scripts
- Arduino will not accept sketch uploads if SMPS is not powered
- ESP32 will not upload if UART cables are still plugged in

<br />

## ROVER

### First Time Setup
- Follow "Setting up..." instructions within the Command_Subsystem folder for either local or AWS server
- Follow "Drive_Control_integration.ino" instructions within the Drive_Subsystem folder to setup APIs for the Arduino
- Configure `config.h` in `/path/Control_Subsystem/Control_program/control_program` using the instructions within Control_Subsystem folder


### Rover Setup
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

### Sending Instructions to Rover
1. Navigate to the website `npm start` opened
2. Login using the appropriate credentials found within the Command_Subsystem folder
3. Select Discrete or Coordinate mode from the top menu
4. Send instructions using boxes the provided fields
