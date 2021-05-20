#include "Orientation.h"

float Direction::getMagnitude() {
    return sqrt(pow(x,2)+pow(y,2));
}


Orientation::Orientation() {

    lastPosition.x = 0; lastPosition.y = 0;
    currentPosition.x = 0; currentPosition.y = 0;

    lastDirection.x = 0; lastDirection.y = 0;
    currentDirection.x = 0; currentDirection.y = -1;

    initial.x = 0; initial.y = -1;
}

void Orientation::updatePosition(int x, int y) {

    //find travel distance
    int travel_distance = sqrt(pow(x-currentPosition.x, 2) + pow(y-currentPosition.y, 2));

    if (travel_distance >= 5) {

        lastPosition = currentPosition;
        currentPosition.x = x; currentPosition.y = y;
        enable_direction_update = 1;
        position_changed = 1;
        getDisplacement();
        detectRotation();
    }

    else {
        lastPosition = lastPosition;
        currentPosition = currentPosition;
        enable_direction_update = 0;
        position_changed = 0;
    }
}

void Orientation::updateDirection() {

    //find magnitudes of current direction and displacement vectors
    float absDisp = displacement.getMagnitude();
    float absDir = currentDirection.getMagnitude();

    //find cosine and sine of direction change angle
    float cosine = 1 - pow(absDisp, 2) / (2 * pow(radius, 2));
    float sine = sqrt(1 - pow(cosine, 2));

    

    //calculate new direction
    Direction newDir;
    
    switch (rotation) {

    case 0:
        //Straight motion
        newDir = currentDirection;
        break;

    case 1:
        //Anticlockwise rotation
        newDir.x = currentDirection.x * cosine - currentDirection.y * sine;
        newDir.y = currentDirection.x * sine + currentDirection.y * cosine;
        break;

    case 2:
        //Clockwise rotation
        newDir.x = currentDirection.x * cosine + currentDirection.y * sine;
        newDir.y = currentDirection.y * cosine - currentDirection.x * sine;
        break;

    default:
        break;
    }

    if (enable_direction_update) {
        lastDirection = currentDirection;
        currentDirection = newDir;

    }
    else {
        lastDirection = lastDirection;
        currentDirection = currentDirection;
    }

}

void Orientation::logOrientation() {
    if(log_enable) {

        Serial.println("Rover Position: " + String(currentPosition.x) + " : " + String(currentPosition.y));
        Serial.print("Rover Direction: "); Serial.print(currentDirection.x, 5); Serial.print(" : "); Serial.println(currentDirection.y, 5);
        Serial.println(" ");
    }
}

void Orientation::enableLog() {
    log_enable = 1;
}

void Orientation::disableLog() {
    log_enable = 0;
}

void Orientation::getDisplacement() {
    displacement.x = currentPosition.x - lastPosition.x;
    displacement.y = currentPosition.y - lastPosition.y;
}

 void Orientation::detectRotation() {
    
    //calculate cross product between currentDirection and displacement
    float crossprod = initial.x * displacement.y - displacement.x * initial.y;
    float absDisp = displacement.getMagnitude();
    float absDir = initial.getMagnitude();
    
    //calculate sine between vectors
    float sine = crossprod / (absDir * absDisp);

    if (sine > 0.4) { rotation = 1; }
    else if (sine < -0.4) { rotation = 2; }
    else { rotation =  0; }

}


 void Orientation::logRotation() {

     switch (rotation) {

     case 0:
         Serial.println("Straight Motion\n");
         break;

     case 1:
         Serial.println("Anticlockwise Rotation\n");
         break;

     case 2:
         Serial.println("Clockwise Rotation\n");
         break;

     default:
         break;
     }
}

 float Orientation::getTravelDistance() {
     if (position_changed) { return displacement.getMagnitude(); }
     else { return 0; }
 }