#include "DriveInterface.h"

#define waitInterval 50

DriveInterface di(&Serial1);

int mDrive_Mode = 0;
int mDistance = 0;
int mSpeed = 0;
int mDirection = 0;

int rover_heading = 0;
int alert = 0;
int axisX = 0;
int axisY = 0;

void setup() {
  di.setBaudrate(38400);
  di.begin();
  Serial.begin(38400);
  Serial.setTimeout(5);
}

void loop() {

  //Command: drive_mode,distance,speed,direction
 
 if (Serial.available())
  {
    char message[30];
    int amount = Serial.readBytesUntil(';', message, 30);
    message[amount] = NULL;

    int data[10];
    int count = 0;
    char* offset = message;
    while (true)
    {
      data[count++] = atoi(offset);
      offset = strchr(offset, ',');
      if (offset) offset++;
      else break;
    }

    mDrive_Mode = data[0];
    mDistance = data[1];
    mSpeed = data[2];
    mDirection = data[3];
    
    di.writeDriveMode(mDrive_Mode);
    di.writeDistance(mDistance);
    di.writeSpeed(mSpeed);
    di.writeDirection(mDirection);
    di.sendUpdates();
  }

  if(di.fetchData())  {
    rover_heading = di.getRoverHeading();  //fetch new direction value
    alert = di.getAlert();  //fetch new speed value
    axisX = di.getAxisX();  // fetch new distance value
    axisY = di.getAxisY(); //fetch new drive mode value

    Serial.println("Heading: " + String(rover_heading));
    Serial.println("Alert: " + String(alert));
    Serial.println("X: " + String(axisX));
    Serial.println("Y: " + String(axisY));
    Serial.println(" ");

  }
}
