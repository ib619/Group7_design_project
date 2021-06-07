#include "RoverFunctions.h"

static const char* TAG = "MyModule";

void initWiFi(const char *ssid, const char *password)    {
    ESP_LOGD(TAG, "Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid,password);
    while(WiFi.status()!=WL_CONNECTED)  {
        delay(100);
    }
    ESP_LOGD(TAG, "Connected! :D");
}

int connectMQTT(PubSubClient *client, const char *mqtt_user, const char * mqtt_password)    {
    ESP_LOGD(TAG, "Connecting to MQTT broker...");
    while(!client->connected()) {
        if(client->connect("RoverESP32", mqtt_user, mqtt_password)) {
            client->subscribe("drive/discrete");
            client->subscribe("drive/t2c");
            client->subscribe("reset");
        }
        else    {
            // do nothing?
        }
    }
    ESP_LOGD(TAG, "MQTT broker connected!");
}

Obstacle cartesianToObstacle(RoverDataStructure *rover, ColourObject co, int index)    {
    double c_x = (co.x - 320)/20;
    double c_y = ((480 - co.y)/20*8)+20;

    double angle = atan2(c_x, c_y) + ((double)rover->rover_heading*DEG_TO_RAD); //in radians
    double distance = sqrt(sq(c_x) + sq(c_y));

    Obstacle tmp;
    tmp.x = (int)((distance*10*sin(angle)) + rover->x_axis);
    tmp.y = (int)((distance*10*cos(angle)) + rover->y_axis);
    tmp.distance = (int)distance;

    switch(index)   {
        case 0:
            tmp.colour="red";
            break;
        case 1:
            tmp.colour="green";
            break;
        case 2:
            tmp.colour="blue";
            break;
        case 3:
            tmp.colour="grey";
            break;
        case 4:
            tmp.colour="yellow";
            break;
        default:
            tmp.colour="unknown";
            break;
    }
    ESP_LOGE(TAG, "Obstacle-> x: %d  y: %d  index: %d", tmp.x, tmp.y, index);
    return tmp;
}

// Obstacle convertObjectToObstacle(RoverDataStructure *rover, ColourObject co, int index) {
//     Obstacle tmp;
//     int angle=rover->rover_heading + co.angle;      // this is in DEGREES, cos/sin function takes in RADIANS
//     tmp.x = (co.distance*10*sin(angle*DEG_TO_RAD)) + rover->x_axis;
//     tmp.y = (co.distance*10*cos(angle*DEG_TO_RAD)) + rover->y_axis;
//     switch(index)   {
//         case 0:
//             tmp.colour="red";
//             break;
//         case 1:
//             tmp.colour="green";
//             break;
//         case 2:
//             tmp.colour="blue";
//             break;
//         case 3:
//             tmp.colour="grey";
//             break;
//         case 4:
//             tmp.colour="yellow";
//             break;
//         default:
//             tmp.colour="unknown";
//             break;
//     }
//     return tmp;
// }

int publishPosition(PubSubClient *client, RoverDataStructure *data)   {
    StaticJsonDocument<256> doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["x"] = data->x_axis;
    obj["y"] = data->y_axis;
    obj["heading"] = data->rover_heading;
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    if(client->publish("position/update", buffer, n) == true)   {
        return 1;
    }
    else    {
        return 0;
    }
}

int publishBatteryStatus(PubSubClient *client, int cell, int soc, int soh, int state)   {
    StaticJsonDocument<256> doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["cell"] = cell;
    obj["battery_level"] = soc;
    obj["battery_soh"] = soh;
    obj["battery_state"] = state;
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    if(client->publish("battery/status", buffer, n)==true)  {
        return 1;
    }
    else    {
        return 0;
    }
}

int publishRoverStatus(PubSubClient *client, RoverDataStructure *data)  {
    StaticJsonDocument<256> doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["drive_status"] = data->alert;
    obj["range"] = data->rover_range;
    obj["obstacle_detected"] = data->obstacle_detected;
    obj["distance_travelled"] = data->total_distance;
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    if(client->publish("rover/status", buffer, n)==true)    {
        return 1;
    }
    else    {
        return 0;
    }
}

int publishRSSI(PubSubClient *client, long rssi)    {
    String s(rssi);
    char buffer[10];
    s.toCharArray(buffer, 10);
    if(client->publish("rssi", buffer, 10)==true)   {
        return 1;
    }
    else    {
        return 0;
    }
}

int publishObstacle(PubSubClient *client, Obstacle obs)    {
    StaticJsonDocument<256> doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["colour"] = obs.colour;
    obj["x"] = obs.x;
    obj["y"] = obs.y;
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    if(client->publish("obstacle/update", buffer, n)==true) {
        return 1;
    }
    else    {
        return 0;
    }
}