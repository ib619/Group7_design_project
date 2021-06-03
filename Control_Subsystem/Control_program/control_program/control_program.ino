#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "RoverFunctions.h"
#include "DriveInterface.h"
#include "EnergyInterface.h"
#include "FPGAInterface.h"
#include "config.h"

#define BASE_ADDRESS 0x40000
#define FPGA_I2C_ADDRESS 0x55

#define COLLISION_THRESHOLD 20  // currently in cm cos object distance comes in cm

#define RSSI_UPDATE_INTERVAL 100
#define VISION_UPDATE_INTERVAL 500

// WiFi stuff
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// mqtt settings
const char* mqttServer = MQTT_SERVER; // MQTT server IP
const char* mqttUser = MQTT_USER;
const char* mqttPassword = MQTT_PASSWORD;
const int mqttPort = MQTT_PORT;

// some objects and data structures
WiFiClient espClient;
PubSubClient mqtt(espClient);
DriveInterface drive(&Serial);
EnergyInterface energy(&Serial1);
FPGAInterface fpga(&Wire);
RoverDataStructure rover;
RoverDataStructure command_holder;

Obstacle obstacles[5];

// flags
int updateFlag=0;
int busyFlag=0;
int obstacleDetectFlag=0;
int collisionFlag=0;

unsigned long ptime=0;
unsigned long vision_ptime=0;
int vision_update=0;

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,"drive/discrete")==0)  {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
    if(busyFlag==0&&collisionFlag==0) {
      rover.direction=doc["direction"];
      rover.distance=doc["distance"];
      rover.speed=doc["speed"];
      rover.drive_mode=1;
      updateFlag=1;
    }
  }
  else if(strcmp(topic,"drive/t2c")==0) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
    if(busyFlag==0&&collisionFlag==0) {
      rover.target_x=doc["x"];
      rover.target_y=doc["y"];
      rover.speed=doc["speed"];
      rover.drive_mode=2;
      updateFlag=1;
    }
  }
  else if(strcmp(topic,"reset")==0) {
    int a=atoi((char*)payload);
    if(a==1) {
      rover.reset=1;
      updateFlag=1;
    }
    else if(a==2)  {
      rover.drive_mode=0;
      updateFlag=1;
    }
  }
}

void setup() {
  // put your setup code here, to run once:  
  fpga.setBusFrequency(1000000);
  fpga.setSlaveAddress(FPGA_I2C_ADDRESS);
  fpga.setBaseAddress(BASE_ADDRESS);
  fpga.begin(GPIO_NUM_13, GPIO_NUM_12);
  fpga.offLEDs();
  
  initWiFi(ssid,password);
  fpga.writeLED(9,1);

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqtt_callback);
  
  drive.setBaudrate(115200);
  drive.begin();

  energy.setBaudrate(38400);
  energy.begin();
  
  fpga.writeLED(7,1);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(WiFi.status()==WL_CONNECTED) {
    if(mqtt.connected()) { // MQTT broker connected
      if(drive.fetchData()) { // check if data available from Drive arduino
        rover.obstacle_detected=drive.getObstacle();
        rover.alert=drive.getAlert();
        rover.x_axis=drive.getAxisX();
        rover.y_axis=drive.getAxisY();
        rover.rover_heading=drive.getRoverHeading();
        
        // send rover data to MQTT broker
        publishPosition(&mqtt, &rover);
        publishRoverStatus(&mqtt, &rover);

        // send position data to Energy Arduino
        energy.writeSpeed(rover.speed);
        energy.writePositionX(rover.x_axis);
        energy.writePositionY(rover.y_axis);
        energy.sendUpdates();

        busyFlag=rover.alert;
        fpga.writeLED(6,busyFlag);
      }

      if(energy.fetchData())  { //check if data available from Energy arduino
        rover.rover_range=energy.getRoverRange();
        rover.battery_state=energy.getState();
        for(int i=0;i<3;i++)  { //send cell info (3 cells) to MQTT broker
          publishBatteryStatus(&mqtt,i,energy.getBatterySOC(i),energy.getBatterySOH(i),rover.battery_state);
        }
      }

      if(millis()-vision_ptime>=VISION_UPDATE_INTERVAL)  {  //throttle vision update
        vision_update=1;        
      }
      ColourObject obj;             // fetch data from FPGA
      for(int i=0;i<5;i++)  {
        obj=fpga.readByIndex(i);
        if(obj.detected>0)  {
          if(obj.distance>0&&collisionFlag==0&&obj.distance<COLLISION_THRESHOLD)  {
            collisionFlag=1;
          }
          if(vision_update) {
            Obstacle obs = convertObjectToObstacle(&rover, obj, i);
            publishObstacle(&mqtt, obs);
          }
        }
      }
      if(vision_update) {
        vision_ptime=millis();
        vision_update=0;
      }

      if(millis()-ptime>=RSSI_UPDATE_INTERVAL)  { // publish RSSI
        publishRSSI(&mqtt, WiFi.RSSI());
        ptime=millis();
      }

      mqtt.loop();    //check for messages from Command
      if(collisionFlag>0) {   //collision detected
        switch(collisionFlag) {
          case 1:
            fpga.writeLED(0,1);
            if(busyFlag==0) {
              collisionFlag=2;
            }
            else  {
              if(rover.drive_mode==2) { //save target coordinates/speed if in t2c mode
                command_holder.drive_mode=2;
                command_holder.target_x=rover.target_x;
                command_holder.target_y=rover.target_y;
                command_holder.speed=rover.speed;
              }
              rover.drive_mode=0;
              updateFlag=1;
            }
            break;
           case 2:              //turn right 90 degrees then move 250mm
            rover.drive_mode=1;
            rover.direction=90;
            rover.speed=150;
            rover.distance=250;
            updateFlag=1;
            collisionFlag=3;
            break;
           case 3:
            if(busyFlag==1) {   //wait for the rover to start moving
              collisionFlag=4;
            }
            break;
           case 4:
            if(busyFlag==0) { //turn left 90 degrees then move 400mm
              rover.drive_mode=1;
              rover.direction=-90;
              rover.speed=150;
              rover.distance=400;
              updateFlag=1;
              collisionFlag=5;
            }
            break;
            case 5:
              if(busyFlag==1) {
                collisionFlag=6;
              }
              break;
             case 6:
              if(busyFlag==0) {   //exit collision avoidance routine
                fpga.writeLED(0,0);
                if(command_holder.drive_mode==2)  { //restore t2c if available
                  rover.drive_mode=2;
                  rover.target_x=command_holder.target_x;
                  rover.target_y=command_holder.target_y;
                  rover.speed=command_holder.speed;
                  updateFlag=1;
                  command_holder.drive_mode=0;
                  command_holder.target_x=0;
                  command_holder.target_y=0;
                  command_holder.speed=0;
                }
                collisionFlag=0;
              }
              break;
             default: //sinkhole
              break;
        }
      }
      if(updateFlag==1) { // send data to drive arduino
        if(rover.speed==0)  {
          rover.drive_mode=0;
        }
        if(rover.reset==0)  { //update control fields only if reset is 0
          drive.writeDriveMode(rover.drive_mode);
          drive.writeDirection(rover.direction);
          drive.writeSpeed(rover.speed);
          drive.writeDistance(rover.distance);
          drive.writeTargetX(rover.target_x);
          drive.writeTargetY(rover.target_y);
        }
        drive.writeReset(rover.reset);
        drive.writeSystemTime(millis());
        drive.sendUpdates();
        updateFlag=0; //clear updateFlag
        if(rover.reset!=0)  {
          rover.reset=0;  //clear rover.reset field if it is not 0
        }
      }
            
      if(rover.drive_mode==1) {   //drive_mode indicator lights
        fpga.writeLED(1,1);
        fpga.writeLED(2,0);
      }
      else if(rover.drive_mode==2)  {
        fpga.writeLED(1,0);
        fpga.writeLED(2,1);
      }
      else  {
        fpga.writeLED(1,0);
        fpga.writeLED(2,0);
      }
    }
    else  { // MQTT broker not connected
      fpga.writeLED(8,0);
      connectMQTT(&mqtt, mqttUser, mqttPassword);
      if(mqtt.connected())  {
        fpga.writeLED(8,1);
      }
    }
  }
  else  {   //WiFi disconnected
    fpga.writeLED(8,0);
    fpga.writeLED(9,0);
    fpga.writeLED(6,0);
    rover.drive_mode=0;
    drive.sendUpdates();
    initWiFi(ssid,password);
    fpga.writeLED(9,1);
    connectMQTT(&mqtt, mqttUser, mqttPassword);
    if(mqtt.connected())  {
      fpga.writeLED(8,1);
    }
  }
}
