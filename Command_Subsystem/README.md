# Command Subsystem

## Setting up ESP32 on Arduino IDE
1. Need to download ESP32 board onto Arduino IDE
2. Select 3MB SPIFF partition
3. `PubSubClient` for the MQTT protocol and `WiFi` for setting up the esp32 as a station
4. `ArduinoJson` for converting MQTT payload to/from JSON

## Setting up the MQTT broker and connections (if running local broker)
1. Install the mosquitto broker using your local package manager
2. In the `mosquitto.conf` file, add the following lines:
```
listener 1883
protocol mqtt
listener 8080
protocol websockets

allow_anonymous true
```
3. On mac, the config file is located at `/usr/local/etc/mosquitto/mosquitto.conf`
4. To start the local mosquitto broker with the config file run: `/usr/local/sbin/mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf -v`

## How to run the frontend (react)
1. Install npm using your local package manager 
2. Run `npm install` to install all the required dependencies in the `client` directory
3. To start the web app `npm start`
    - This will start the development build for the client (react app)

## How to run the backend (python)
1. In the `server` directory, run `pip3 install -r requirements.txt`
2. To create the `.db` file, run `python3 database.py` in the `server` directory (this is only if you haven't already created the database)
3. To run the backend server, run `python3 mqtt_server.py` in the `server` directory
4. Can also run the script `./start_server.sh` in the `server` directory to set up the database and server
    - remember to `chmod +x start_server.sh` first

## How to run production build
1. `npm run build` to create `build` folder
2. `serve -s build` to view the webpage on your local network

## Details
1. Things to send back to ESP32
    - Direction (angle from current facing direction, clockwise)
    - Speed
    - Distance

2. Things to receive from ESP32:
    - Current coordinates (relative to start)
    - Distance travelled since the start 
    - Battery state (state of charge)
    - Battery health (state of health)
    - State of rover (returning back or exploring)
    - Signal Strength

3. Backend/Database:
    - Backend database using sql hosted on AWS using Docker containers
    - MQTT protocol for communication between esp32, backend and frontend
