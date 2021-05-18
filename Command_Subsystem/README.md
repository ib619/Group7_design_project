# rover-command

# Setting up ESP32 on Arduino IDE
1. Need to download ESP32 board onto Arduino IDE
2. Select 3MB SPIFF partition

[This link for serving react app on ESP](https://techtutorialsx.com/2019/03/24/esp32-arduino-serving-a-react-js-app/)

# React App
1. Things to send back to ESP32
    - Direction vector (angle from current facing direction)
    - Speed

2. Things to receive from ESP32:
    - Current coordinates (relative to start)
    - Distance travelled since the start 
    - Battery state (state of charge)
    - Battery health (state of health)
    - State of rover (returning back or exploring)

3. Processing to do on web:
    - Path finding algorithm to go back to charging station
    - Display top 5 newest obstacle coordinates in different transparency
    - MongoDB storing of all the obstacles coordinates over time 