#include "EnergyInterface.h"

EnergyInterface::EnergyInterface(HardwareSerial *ser)   {
    serial=ser;
}

int EnergyInterface::begin()    {
    ser->begin(baudrate, SERIAL_8N1, RX_PIN, TX_PIN);
    ser->setTimeout(timeout);
    return 1;
}

void EnergyInterface::setBaudrate(long brate)   {
    baudrate=brate;
}

void EnergyInterface::setTimeout(long tm)   {
    timeout=tm;
}

int EnergyInterface::fetchData()    {
    if(ser->available()>=DATA_PACKET_SIZE)  {
        int tmp[DATA_PACKET_SIZE/2];
        char high_byte;
        char low_byte;
        for(int i=0;i<DATA_PACKET_SIZE/2;i++)   {
            high_byte=serial->read();
            low_byte=serial->read();
            tmp[i]=(high_byte<<8)+low_byte;     //decoding as unsigned
        }
        for(int i=0;i<3;i++)    {
            battery_soc[i]=tmp[2*i];
            battery_soh[i]=tmp[2*i+1];
        }
        state=tmp[6];
        rover_range=tmp[7];
        return 1;
    }
    else    {
        return 0;
    }
}

void EnergyInterface::sendUpdates() {
    send_integer(command);
    send_integer(speed);
    send_integer(pos_x);
    send_integer(pos_y);
}

int EnergyInterface::getBatterySOC(int cell) const  {
    return battery_soc[cell];
}

int EnergyInterface::getBatterySOH(int cell) const  {
    return battery_soh[cell];
}

int EnergyInterface::getState() const   {
    return state;
}

int EnergyInterface::getRoverRange() const  {
    return rover_range;
}

void EnergyInterface::writeCommand(int com) {
    command=com;
}

void EnergyInterface::writeSpeed(int spd)   {
    speed=spd;
}

void EnergyInterface::writePositionX(int x) {
    pos_x=x;
}

void EnergyInterface::writePositionY(int y) {
    pos_y=y;
}

void EnergyInterface::send_integer(int d)  {   // send integer MSB first
    serial->write(d>>8);
    serial->write(d&255);
}