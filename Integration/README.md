## DEPENDENCIES

### List of non-standard Arduino libraries needed for final rover:
1. INA219_WE (Download from Arduino library manager)
2. ESP32 API (https://github.com/espressif/arduino-esp32) 
3. PubSubClient (Download from Arduino library manager)
4. ArduinoJson (https://github.com/bblanchon/ArduinoJson)
5. Arduino MegaAVR Boards (Download from Arduino library manager)

### Tools needed to configure final rover:
1. Arduino IDE 1.8.15 (https://www.arduino.cc/en/software)
2. Quartus 16.1 (https://fpgasoftware.intel.com/16.1/)
3. nios2 (comes with Quartus 16.1 installation)
4. npm (https://www.npmjs.com/get-npm)
5. Python 3.X.X (https://www.python.org/downloads/)
6. Mosquitto MQTT Broker v2.0.10 (https://mosquitto.org/download/)

<br />

## ARDUINO IDE SETTINGS

### ESP32 Settings:
- Board Name/Type: ESP32 Dev Module
- Partition: HUGE APP

### Arduino Settings:
- Board Name/Type: Arduino Nano Every

### Known problems with uploading scripts
1. Arduino will not accept sketch uploads if SMPS is not powered
2. ESP32 will not upload if UART cables are still plugged in

<br />

## ROVER

### First Time Setup
1. Navigate to Mosquitto MQTT Broker installation folder
2. In the `mosquitto.conf` file, add the following lines:
```
listener 1883
protocol mqtt
listener 8080
protocol websockets

allow_anonymous true
```
3. Navigate to /path/Command_Subsystem/client
4. Run in terminal: npm install


### Rover Setup
1. Open Mosquitto with: mosquitto.exe -c mosquitto.conf -v
2. Run 'npm start' within /path/Command_Subsystem/client
3. Run 'start_server.sh' within /path/Command_Subsystem/server
4. Upload the .sof file found in /path/Integration using the Quartus Programmer
5. Upload the .elf file found in /path/Integration using nios2
6. Upload the Arduino file found in /path/Integration/Arduino using the Arduino IDE with the correct settings
7. Unplug the UART cables from the ESP32
8. Upload the ESP32 file found in /path/Integration/ESP322 using the Arduino IDE with the correct settings
9. Plug in the UART cables again


### Sending Instructions to Rover
1. Navigate to the website 'npm start' opened
2. Select Discrete or Coordinate mode from the top menu
3. Enter in desired coordinates/distance/direction/speed
