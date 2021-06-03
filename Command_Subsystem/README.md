# Command Subsystem

## Setting up ESP32 on Arduino IDE
1. Need to download ESP32 board onto Arduino IDE
2. Select 3MB SPIFF partition
3. `PubSubClient` for the MQTT protocol and `WiFi` for setting up the esp32 as a station
4. `ArduinoJson` for converting MQTT payload to/from JSON

## Setting up the MQTT broker and connections (if running local broker)
1. Install the mosquitto broker using your local package manager
2. Copy the acl and pwdfile from `server` into the mosquitto directory
3. In the `mosquitto.conf` file, add the following lines:

```
listener 1883
protocol mqtt
listener 8080
protocol websockets

allow_anonymous false
password_file path to pwdfile]
acl_file [path to acl file]
```
4. On mac, the config file is located at `/usr/local/etc/mosquitto/mosquitto.conf`
5. To start the local mosquitto broker with the config file run: `/usr/local/sbin/mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf -v`
    - [path to mosquitto exe] -c [path to config file]

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
    - `chmod +x start_server.sh` first

## How to run production build
1. `npm run build` to create `build` folder
2. `serve -s build` to view the webpage on your local network

## How to set up AWS server
1. Create EC2 Ubuntu Server and open the following ports:
    - Custom TCP 8080 (Used for websockets with React, May not need to open this if React is on EC2 itself)
    - Cusom TCP 1883 (Used for MQTT)
    - SSH 22
2. Specify "Anywhere" for the Source so that connections can be made anywhere
3. After downloading the key-pair, run `chmod og-rwx [path to .pem file]` to change permissions
4. To ssh into the instance, run `ssh -i [path to key-pair] ubuntu@[ip address of server]
5. To setup the stuff, in the root directory
```
sudo apt update
[ clone the git repo ]
cd Group7_design_project/Command_Subsystem/server
sudo curl -sSL https://get.docker.com/ | sh
sudo docker build . -t mars_server:1.0 --file Dockerfile 
```
6. Run this command to start up the docker image and link docker ports to server ports
`sudo docker run -it --rm -p 0.0.0.0:1883:1883 -p 0.0.0.0:8080:8080/tcp mars_server:1.0`
## Details
1. Things to send back to ESP32
    - drive/discrete: discrete mode instructions 
    - drive/t2c: t2c mode instructions
    - reset: reset rover to 0,0 / emergency stop
    - battery/recalibrate: tell battery to recalibrate

2. Things to receive from ESP32:
    - postiion/update: rover's current position
    - obstacle/result: 5 most recent obtsacle data
    - rover/status: rover status / remaining range
    - battery/status: battery soh / soc / state
    - path: past positions to map out path taken by rover
    - rssi: signal strength

3. Backend/Database:
    - Backend database using sql hosted on AWS using Docker containers
    - MQTT protocol for communication between esp32, backend and frontend
    - obstacle/update: stream of detected obstacle position, keeps track of recent ones and stores in a db
    - position/update: stores positions in a db to be sent on path 