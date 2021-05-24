#!/bin/bash
mkdir db
python3 database.py
python3 mqtt_server.py