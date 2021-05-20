This folder contains the Drive API and sample Arduino code for driving forward.

To use Arduino code: 
    1) latest version of Orientation API has to be installed
    2) latest version of Drive API has to be installed

Instructions for sample code operation:
    1) type the following command into Serial: Distance,Speed,Direction;

        - Distance is any positive distance in millimeters
        - Speed is from 0 minimum speed, to 255 maximum speed
        - Direction has to be 0 for this version(as turning is not yet implemented)

    2) once rover has executed your command, type in the next one
    3) be carefull not to touch the rover when it is stationary as it may break the orientation algorithm


OBJECTIVES:

    1) Design the turning algorithm
    2) Integrate with ESP32->Arduino API
    3) Design the direction vector based anti-drift system
                    