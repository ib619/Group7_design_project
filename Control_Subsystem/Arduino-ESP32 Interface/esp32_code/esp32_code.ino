#include "DriveInterface.h"

#define waitInterval 50

DriveInterface di(&Serial);

void setup() {
  di.setBaudrate(38400);
  di.begin();
}

void loop() {
  // varies the speed between 60 and 254
  for(int i=60;i<255;i++) {
    di.writeDirection(0);
    di.writeSpeed(i);    
    di.writeSystemTime(millis());
    di.sendUpdates();
    delay(waitInterval);
  }
  for(int i=254;i>=60;i--)  {
    di.writeDirection(0);
    di.writeSpeed(i);
    di.writeSystemTime(millis());
    di.sendUpdates();
    delay(waitInterval);
  }
}
