#ifndef CONTROL_INTERFACE_API_H
#define CONTROL_INTERFACE_API_H

#include <Arduino.h>

class ControlInterface {
    public:
        ControlInterface(HardwareSerial *ser);
        int begin();
        int fetchData();   // returns 0 if no updates, 1 if there is new data available
        void sendUpdates(); // send data values
        void setBaudrate(long brate); // default baudrate 115200
        void setTimeout(long tm);

        int getDriveMode() const;
        int getDirection() const;
        int getSpeed() const;
        int getDistance() const;

        void writeBatterylevel(int batt);
        void writeRange(int rng);
        void writeObstacle(int obs);
        void writeAlert(int alrt);
    
    private:
        void send_integer(int data);

        HardwareSerial *serial;
        long baudrate=115200;
        long serial_timeout=10;  // serial port timeout in milliseconds

        int control_packet_size=8;  // multiple of 2 as all data comes in int
        int ctrl_size;

        // control values
        int drive_mode=0;
        int direction=0;
        int speed=0;
        int distance=0;

        // data values
        int battery_level=0;
        int rover_range=0;
        int obstacle_detected=0;
        int alert=0;
};

#endif