#include "ControlInterface.h"

// create instance of control interface running on port "Serial"
ControlInterface ci(&Serial);

// variables for direction, speed and distance
int direction_value=0;
int speed_value=0;
int distance_value=0;
unsigned long sys_time=0;

// variable for data to send to ESP32
int battery_level=100;
int rover_range=100;
int obstacle_detected=0;
int alert=0;
int x_axis=0;
int y_axis=0;

void setup() {
  // setup and start control interface
  ci.setBaudrate(38400);
  ci.setTimeout(5);
  ci.begin();
  delay(3000);
  ci.flushReadBuffer();
}

void loop() {
  // check for new data and update respective variables
  if(ci.fetchData())  {
    direction_value=ci.getDirection();  //fetch new direction value
    speed_value=ci.getSpeed();  //fetch new speed value
    distance_value=ci.getDistance();  // fetch new distance value
    sys_time=ci.getSystemTime();
  }

  // write new values to interface module
  ci.writeBatterylevel(battery_level);
  ci.writeRange(rover_range);
  ci.writeObstacle(obstacle_detected);
  ci.writeAlert(alert);
  ci.writeAxisX(x_axis);
  ci.writeAxisY(y_axis);
  ci.sendUpdates(); //send new values to ESP32

}
