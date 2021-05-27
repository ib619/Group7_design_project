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