This folder contains the Drive API and sample Arduino code for driving and turning.

To use Arduino code(drive_and_turn.ino): 
    1) latest version of Orientation API has to be installed
    2) latest version of Drive API has to be installed

Instructions for sample code operation:
    1) type the following command into Serial: Distance,Speed,Direction;

        - Distance is any positive distance in millimeters
        - Speed is from 0 minimum speed, to 255 maximum speed
        - Direction specifies the turn angle and has the range -180:180 where:
             -180->1 is turning anticlockwise
              0 is moving forward
              1->180 is turning clockwise 

    2) once rover has executed your command, type in the next one
    3) be carefull not to touch the rover when it is stationary as it may break the orientation algorithm


OBJECTIVES:

    1) Integrate with ESP32->Arduino API
    2) Design the direction vector based anti-drift system
    3) Experiment with alternative turning method involving two wheels rotating in different directions                    