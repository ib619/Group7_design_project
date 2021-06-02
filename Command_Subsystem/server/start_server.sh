#!/bin/bash
mkdir db
mosquitto -d -c /etc/mosquitto/mosquitto.conf
python3 database.py
python3 mqtt_server.py