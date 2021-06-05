#include "Orientation.h"

float Direction::getMagnitude() {
    return sqrt(pow(x,2)+pow(y,2));
}


Orientation::Orientation() {

    lastPosition.x = 0; lastPosition.y = 0;
    currentPosition.x = 0; currentPosition.y = 0;
    truePosition.x = 0; truePosition.y = 0;

    lastDirection.x = 0; lastDirection.y = 1;
    currentDirection.x = 0; currentDirection.y = 1;

    initial.x = 0; initial.y = 1;
}

void Orientation::updatePosition(int x, int y, bool FW, bool BW, bool CW, bool ACW) {

    //find travel distance
    int travel_distance = sqrt(pow(x-currentPosition.x, 2) + pow(y-currentPosition.y, 2));

    if (travel_distance >= 2) {

        total_distance_travelled += travel_distance;

        lastPosition = currentPosition;
        currentPosition.x = x; currentPosition.y = y;

        enable_direction_update = 1;
        position_changed = 1;
        getDisplacement();
        detectRotation();

        bool forward_condition = currentPosition.x > lastPosition.x || currentPosition.y > lastPosition.y;
        bool backward_condition = currentPosition.x < lastPosition.x || currentPosition.y < lastPosition.y;

        bool enable_change = (!ACW && !CW) && (FW || BW);

        if (enable_change && forward_condition) {
            truePosition.x = truePosition.x + currentDirection.x * displacement.getMagnitude();
            truePosition.y = truePosition.y + currentDirection.y * displacement.getMagnitude();
        }
        else if (enable_change && backward_condition) {
            truePosition.x = truePosition.x - currentDirection.x * displacement.getMagnitude();
            truePosition.y = truePosition.y - currentDirection.y * displacement.getMagnitude();
        }
        else {
            truePosition = truePosition;
        }
    }

    else {
        lastPosition = lastPosition;
        currentPosition = currentPosition;
        truePosition = truePosition;
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

        Serial.println("Rover Position: " + String(truePosition.x,5) + " : " + String(truePosition.y,5));
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

    if (sine > 0) { rotation = 2; }
    else if (sine < 0) { rotation = 1; }
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

 float Orientation::getDirectionChangeAngle() {

     float dotprod = lastDirection.x * currentDirection.x + lastDirection.y * currentDirection.y;
     float absLastDir = lastDirection.getMagnitude();
     float absCurDir = currentDirection.getMagnitude();

     float cosine = dotprod / (absLastDir * absCurDir);

     float angle = acos(cosine);

     if (position_changed) { return angle * (180 / 3.14159); }
     else { return 0; }
 }

 int Orientation::exportPositionX() {
     return (int)truePosition.x;
 }

 int Orientation::exportPositionY() {
     return (int)truePosition.y;
 }

 int Orientation::exportDirectionAngle() {
     //calculate cross product between initial and current directions
     float crossprod = currentDirection.x * initial.y - initial.x * currentDirection.y;
     float dotprod = currentDirection.x * currentDirection.y + initial.x * initial.y;
     float absDisp = currentDirection.getMagnitude();
     float absDir = initial.getMagnitude();

     //calculate sine and cosine of angle between vectors
     float sine = crossprod / (absDir * absDisp);
     float cosine = dotprod / (absDir * absDisp);

     float angle = asin(sine);
     float output;

     if (cosine < 0 && sine >= 0) { output = 180 -angle * (180 / 3.14159); }
     else if (cosine >= 0 && sine >= 0) { output =  angle * (180 / 3.14159); }
     else if (cosine < 0 && sine < 0) { output = angle * (180 / 3.14159); }
     else if (cosine >= 0 && sine < 0) { output = -180 - angle * (180 / 3.14159); }

     return (int)output;
 }

 unsigned long Orientation::exportTotalRun() {
     return total_distance_travelled;
 }

 float Orientation::passPosX() {
     return truePosition.x;
 }

 float Orientation::passPosY() {
     return truePosition.y;
 }

 float Orientation::passDirX() {
     return currentDirection.x;
 }

 float Orientation::passDirY() {
     return currentDirection.y;
 }

 void Orientation::resetData(int trigger) {
     if (trigger == 1) {
         lastPosition.x = 0; lastPosition.y = 0;
         currentPosition.x = 0; currentPosition.y = 0;
         truePosition.x = 0; truePosition.y = 0;

         lastDirection.x = 0; lastDirection.y = 1;
         currentDirection.x = 0; currentDirection.y = 1;

         initial.x = 0; initial.y = 1;
     }
 }