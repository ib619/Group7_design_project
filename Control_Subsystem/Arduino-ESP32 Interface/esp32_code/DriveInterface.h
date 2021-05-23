#ifndef DRIVE_INTERFACE_API_H
#define DRIVE_INTERFACE_API_H

#include <Arduino.h>

#define DATA_PACKET_SIZE 16

class DriveInterface {
    public:
        DriveInterface(HardwareSerial *ser);
        int begin();
        int fetchData();
        void sendUpdates();
        void setBaudrate(long brate);
        void setTimeout(long tm);

        void writeDriveMode(int dm);
        void writeDirection(int dir);
        void writeSpeed(int spd);
        void writeDistance(int dist);
        void writeTargetX(int x);
        void writeTargetY(int y);

        int getBatteryLevel() const;
        int getRange() const;
        int getObstacle() const;
        int getAlert() const;
        int getAxisX() const;
        int getAxisY() const;
        int getRoverHeading() const;
        int getBatterySOH() const;

    private:
        void send_integer(int d);

        HardwareSerial *serial;
        long baudrate=38400;
        long timeout=5;

        int data_size=DATA_PACKET_SIZE/2;

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
};

#endif