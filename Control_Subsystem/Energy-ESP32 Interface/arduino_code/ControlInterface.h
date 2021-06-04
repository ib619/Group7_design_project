#ifndef CONTROL_INTERFACE_ENERGY_API_H
#define CONTROL_INTERFACE_ENERGY_API_H

// control interface for energy arduino

#define CONTROL_PACKET_SIZE 12

#include "Arduino.h"

class ControlInterface {
    public:
        ControlInterface(HardwareSerial *ser);
        int begin();
        int fetchData();
        void sendUpdates();
        void setBaudrate(long br);
        void setTimeout(long tm);
        void flushReadBuffer();

        int getCommand() const;
        int getSpeed() const;
        int getPositionX() const;
        int getPositionY() const;
        unsigned long getTotalDistance() const;

        void writeSOC(int cell, int value);
        void writeSOH(int cell, int value);
        void writeState(int st);
        void writeRoverRange(int range);

    private:
        void send_integer(int data);

        HardwareSerial * serial;
        long baudrate=38400;
        long timeout=10;

        //control values
        int command=0;
        int speed=0;
        int pos_x=0;
        int pos_y=0;
        unsigned long total_distance=0;  // 4 bytes

        // data values
        int battery_soc[3]={0,0,0}; // cell number starts from 0
        int battery_soh[3]={0,0,0};
        int state=0;
        int rover_range=0;
};

#endif
