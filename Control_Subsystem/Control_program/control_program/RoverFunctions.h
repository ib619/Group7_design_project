#ifndef ROVER_FUNCTIONS_API_H
#define ROVER_FUNCTIONS_API_H

#include "Arduino.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "ArduinoJson.h"
#include "FPGAInterface.h"

struct Obstacle {
    String colour;
    int x=0;
    int y=0;
};

struct RoverDataStructure {
    // control values
        int drive_mode=0;
        int direction=0;
        int speed=0;
        int distance=0;
        int target_x=0;
        int target_y=0;

        // data values
        int battery_level=0;
        int rover_range=0;
        int obstacle_detected=0;
        int alert=0;
        int x_axis=0;
        int y_axis=0;
        int rover_heading=0;
        int battery_SOH=0;
        int battery_state=0;
};

void initWiFi(const char *ssid, const char *password);
int connectMQTT(PubSubClient *client, const char *mqtt_user, const char * mqtt_password);
Obstacle convertObjectToObstacle(RoverDataStructure *rover, ColourObject co, int index);

int publishPosition(PubSubClient *client, RoverDataStructure *data);
int publishBatteryStatus(PubSubClient *client, RoverDataStructure *data);
int publishRoverStatus(PubSubClient *client, RoverDataStructure *data);
int publishRSSI(PubSubClient *client, long rssi);
int publishObstacle(PubSubClient *client, Obstacle obs);

#endif