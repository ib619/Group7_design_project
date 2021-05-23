# Command Subsystem

## Setting up ESP32 on Arduino IDE
1. Need to download ESP32 board onto Arduino IDE
2. Select 3MB SPIFF partition
3. `PubSubClient` for the MQTT protocol and `WiFi.h` for setting up the esp32 as a station

## Setting up the MQTT broker and connections (if running local broker)
1. In the `mosquitto.conf` file, add the following lines:
```
listener 1883
protocol mqtt
listener 8080
protocol websockets

allow_anonymous true
```
2. On mac, the config file is located at `/usr/local/etc/mosquitto/mosquitto.conf`
3. To start the local mosquitto broker with the config file run: `/usr/local/sbin/mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf -v`

## How to run the app
1. Run `npm install` to install all the required dependencies in the `client` directory
2. To start the web app `npm start`
    - This will start the development build for both the server and the client
    - use `npm run server/client` if you only want to run one of them
3. To run the backend server, run `python3 mqtt_server.py` in the `server` directory
4. To create the `.db` file, run `python3 database.py` in the `server` directory
5. Can also run the script `./start_server.sh` in the `server` directory to set up the database and server

## Details
1. Things to send back to ESP32
    - Direction vector (angle from current facing direction)
    - Speed
    - Distance
    - Drive mode (continuous or discrete mode)

2. Things to receive from ESP32:
    - Current coordinates (relative to start)
    - Distance travelled since the start 
    - Battery state (state of charge)
    - Battery health (state of health)
    - State of rover (returning back or exploring)

3. Backend/Database:
    - Backend database using sql hosted on AWS using Docker containers
    - MQTT protocol for communication between esp32, backend and frontend
