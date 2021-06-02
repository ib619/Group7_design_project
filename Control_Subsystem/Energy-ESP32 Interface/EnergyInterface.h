#ifndef ENERGY_INTERFACE_API_H
#define ENERGY_INTERFACE_API_H

#include "Arduino.h"

#define TX_PIN 27   // A0
#define RX_PIN 26   // A1

#define DATA_PACKET_SIZE 16

class EnergyInterface {
    public:
        EnergyInterface(HardwareSerial *ser);
        int begin();
        int fetchData();
        void sendUpdates();
        void setBaudrate(long brate);
        void setTimeout(long tm);

        int getBatterySOC(int cell) const;
        int getBatterySOH(int cell) const;
        int getState() const;
        int getRoverRange() const;

        void writeCommand(int com);
        void writeSpeed(int spd);
        void writePositionX(int x);
        void writePositionY(int y);
    
    private:
        void send_integer(int data);

        HardwareSerial *serial;
        long baudrate=38400;
        long timeout=10;

        //control values
        int command=0;
        int speed=0;
        int pos_x=0;
        int pos_y=0;

        // data values
        int battery_soc[3]={0,0,0}; // cell number starts from 0
        int battery_soh[3]={0,0,0};
        int state=0;
        int rover_range=0;
};

#endif