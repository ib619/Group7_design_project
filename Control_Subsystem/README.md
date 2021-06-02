#Control Program
The main program for the ESP32 is located in "Control_program/control_program". This code integrates the communication with all sub-modules together into a single program

The folder contains the arduino .ino file along with all the library dependencies. These libraries are the latest versions

To install the ESP32 code, compile and upload the "control_program.ino" to the ESP32

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
### FPGA LED Indicators
LEDR 9: WiFi Connected
LEDR 8: MQTT Broker Connected
LEDR 7: Main Loop entry
LEDR 6: command execution status (on: busy, off: free)
LEDR 2: drive_mode 2 engaged
LEDR 1: drive_mode 1 engaged
LEDR 0: collision avoidance routine engaged

(if LEDR 1 and LEDR 2 are off, drive_mode is 0)
<br>
### Arduino-ESP32 API Master Source files
Drive: Control_Subsystem/Arduino-ESP32 Interface/arduino_code (ControlInterface.h and ControlInterface.cpp)

Energy: Control_Subsystem/Energy-ESP32 Interface/arduino_code (ControlInterface.h and ControlInterface.cpp)