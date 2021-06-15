#ifndef DRIVE_INTERFACE_API_H
#define DRIVE_INTERFACE_API_H

#include <Arduino.h>

#define TX_PIN 27   // A0
#define RX_PIN 26   // A1

#define DATA_PACKET_SIZE 12

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
        void writeSystemTime(unsigned long time);
        void writeReset(int rst);

        // int getBatteryLevel() const;
        // int getRange() const;
        // int getObstacle() const;
        int getAlert() const;
        int getAxisX() const;
        int getAxisY() const;
        int getRoverHeading() const;
        // int getBatterySOH() const;
        // int getBatteryState() const;
        unsigned long getTotalDistance() const;

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
        unsigned long system_time=0;    /// 4 bytes
        int reset=0;

        // data values
        int alert=0;
        int x_axis=0;
        int y_axis=0;
        int rover_heading=0;
        unsigned long total_distance=0;  // 4 bytes
};

#endif