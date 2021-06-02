#include "ControlInterface.h"

ControlInterface::ControlInterface(HardwareSerial *ser) {
    serial=ser;
}

int ControlInterface::begin()   {
    serial->begin(baudrate);
    serial->setTimeout(timeout);
    return 1;
}

void ControlInterface::setBaudrate(long brate)  {
    baudrate=brate;
}

void ControlInterface::setTimeout(long tm)  {
    timeout=tm;
}

void ControlInterface::flushReadBuffer()    {
    while(serial->available()>0)    {
        char c=serial->read();
    }
}

int ControlInterface::fetchData()   {
    if(serial->available()>=CONTROL_PACKET_SIZE)    {
        int tmp[CONTROL_PACKET_SIZE/2];
        char high_byte;
        char low_byte;
        for(int i=0;i<CONTROL_PACKET_SIZE/2;i++)  {
            high_byte=serial->read();
            low_byte=serial->read();
            tmp[i]=(int16_t)((high_byte<<8)+low_byte);
        }
        command=tmp[0];
        speed=tmp[1];
        pos_x=tmp[2];
        pos_y=tmp[3];
        return 1;
    }
    else    {
        return 0;
    }
}

void ControlInterface::sendUpdates()    {
    for(int i=0;i<3;i++)    {
        send_integer(battery_soc[i]);
        send_integer(battery_soh[i]);
    }
    send_integer(state);
}

int ControlInterface::getCommand() const    {
    return command;
}

void ControlInterface::writeSOC(int cell, int value)    { // cell number starts from 0
    battery_soc[cell]=value;
}

void ControlInterface::writeSOH(int cell, int value)    {
    battery_soh[cell]=value;
}

void ControlInterface::writeState(int st)    {
    state=st;
}

void ControlInterface::send_integer(int d)  {   // send integer MSB first
    serial->write(d>>8);
    serial->write(d&255);
}
