# Control Program
The control program runs on the rover's ESP32 and is the bridge between Vision, Drive and Energy subsystems, and Command subsystem. 

This folder contains the main control program along with the control interface APIs for the Drive and Energy subsystems. 

The control program requires a config.h file with the credentials and settings for the WiFi network and MQTT broker (elaborated on later)

### Folders
##### Control_program
This folder contains the Arduino project for the ESP32. The main file is called control_program.ino and the project folder comes with all the custom libraries required. Compile and install this program onto the rover's ESP32 with the following settings:  

Board Settings:
* Board: ESP32 Dev Module
* Partition Scheme: Huge (3MB No OTA/1MB SPIFFS)
* Core Level Debug: None
* PSRAM: Disabled

Required External Library Dependencies:
* PubSubClient (https://github.com/knolleary/pubsubclient)
* ArduinoJson (https://arduinojson.org/)
<br >

**Arduino-ESP32 Interface** <br>
* arduino_code: control interface libraries for the Drive Arduino. Copy these to the same project folder as the Drive program
* esp32_code: drive interface libraries for the Control ESP32. These are already updated in the control_program project folder 
<br>
Library files for Drive Arduino: ControlInterface.h and ControlInterface.cpp<br>
Library files for Control ESP32: DriveInterface.h and DriveInterface.cpp
<br>

**Energy-ESP32 Interface** <br>
This folder contains the libraries for the interface between Energy and Control subsystems
* arduino_code: libraries and ecample code for the Energy Arduino. Copy these to the same project folder as the Drive program 
* The EnergyInterface library files for the Control ESP32 are in this folder. These are already updated in the control_program project folder
<br>
Library files for Energy Arduino: ControlInterface.h and ControlInterface.cpp<br>
Library files for Control ESP32: EnergyInterface.h and EnergyInterface.cpp
<br>

**FPGA-ESP32 Interface** <br>
This folder contains the library files for the ESP32 to interface with the FPGA's DMA. These are already updated in the control_program project folder
<br>
Library files for Control ESP32: FPGAInterface.h and FPGAInterface.cpp
<br>

### config.h file
The control program requires settings for the WiFi network SSID and password as well as IP Address/Domain and credentials for the MQTT broker. These are defined in the "config.h" file which every user needs their own local copy of. 

The local "config.h" file can be created using the "config_template.h" file provided in the repo. The required information fields are: 
1) WIFI_SSID: SSID of the WiFi networj the ESP32 connects to
2) WIFI_PASSWORD: WiFi network password 
3) MQTT_SERVER: IP Address or domain name of the MQTT broker
4) MQTT_PORT: port of MQTT broker (defaults to 1883)
5) MQTT_USER: username for MQTT broker (can leave as default)
6) MQTT_PASSWORD: password for MQTT broker (can leave as default)
<br>

**FPGA LED Indicators**
The FPGA LEDs indicate various rover statuses: 
* LEDR 9: WiFi Connected
* LEDR 8: MQTT Broker Connected
* LEDR 7: Main Loop entry
* LEDR 6: command execution status (on: busy, off: free)
* LEDR 5: rover status
* LEDR 3: obstacle detected
* LEDR 2: drive_mode 2 engaged
* LEDR 1: drive_mode 1 engaged
* LEDR 0: collision avoidance routine engaged

(if LEDR 1 and LEDR 2 are both off, drive_mode is 0)
<br>
### Arduino-ESP32 API Master Source files
Drive: Control_Subsystem/Arduino-ESP32 Interface/arduino_code (ControlInterface.h and ControlInterface.cpp)

Energy: Control_Subsystem/Energy-ESP32 Interface/arduino_code (ControlInterface.h and ControlInterface.cpp)