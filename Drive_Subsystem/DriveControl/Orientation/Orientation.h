#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <Arduino.h>

struct Position {
    int x;
    int y;
};

class Direction {
    public:
        float x;
        float y;
        float getMagnitude();
};

class Orientation {
public:
    Orientation();
    void updatePosition(int x, int y); //updates the position of the rover
    void updateDirection(); //updates the direction of the rover, needs to be called after updatePosition()
    void logOrientation(); //logs Orientation data to Serial for debug
    void enableLog(); //enables logging for debug 
    void disableLog(); //disables logging gor debug
    bool position_changed = 0; //indicates if there was a position change since last loop cycle

private:
    int radius = 176;
    int rotation;
    bool log_enable = 0;
    bool enable_direction_update = 0;
    Position lastPosition; 
    Position currentPosition;
    Direction lastDirection;
    Direction currentDirection;
    Direction displacement;
    Direction initial;
    void detectRotation(); //detects which type of motion has been performed by rover
    void getDisplacement(); //calculates displacement since last loop cycle
    void logRotation(); // logs the type of motion performed by the rover for debug

};

#endif