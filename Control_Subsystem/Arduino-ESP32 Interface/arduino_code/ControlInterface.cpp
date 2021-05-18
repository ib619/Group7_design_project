#include "ControlInterface.h"

ControlInterface::ControlInterface(HardwareSerial *ser)    {
    serial=ser;
}

int ControlInterface::begin()   {
    serial->begin(baudrate);
    serial->setTimeout(serial_timeout);
    return 1;
}

void ControlInterface::setBaudrate(long brate)  {
    baudrate=brate;
}

void ControlInterface::setTimeout(long tm)   {
    serial_timeout=tm;
}

void ControlInterface::flushReadBuffer()    {
    while(serial->available()>0)    {
        char c=serial->read();
    }
}

int ControlInterface::fetchData()  {
    if(serial->available()>=control_packet_size)    {
        int tmp[ctrl_size];
        byte high_byte;
        byte low_byte;
        for(int i=0;i<ctrl_size;i++)    {
            high_byte=serial->read();
            low_byte=serial->read();
            tmp[i]=(high_byte<<8)+low_byte;
        }
        // load received control values into respective variables
        drive_mode=tmp[0];
        direction=tmp[1];
        speed=tmp[2];
        distance=tmp[3];
        return 1;
    }
    else    {
        return 0;
    }
}

void ControlInterface::sendUpdates()    {
    send_integer(battery_level);
    send_integer(rover_range);
    send_integer(obstacle_detected);
    send_integer(alert);
    send_integer(x_axis);
    send_integer(y_axis);
}

int ControlInterface::getDriveMode() const  {
    return drive_mode;
}

int ControlInterface::getDirection() const  {
    return direction;
}

int ControlInterface::getSpeed() const  {
    return speed;
}

int ControlInterface::getDistance() const   {
    return distance;
}

void ControlInterface::writeBatterylevel(int batt) {
    battery_level=batt;
}

void ControlInterface::writeRange(int rng)  {
    rover_range=rng;
}

void ControlInterface::writeObstacle(int obs)   {
    obstacle_detected=obs;
}

void ControlInterface::writeAlert(int alrt) {
    alert=alrt;
}

void ControlInterface::writeAxisX(int x)    {
    x_axis=x;
}

void ControlInterface::writeAxisY(int y)    {
    y_axis=y;
}

void ControlInterface::send_integer(int d)  {   // send integer MSB first
    serial->write(d>>8);
    serial->write(d&255);
}