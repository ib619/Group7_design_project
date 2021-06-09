#include "DriveInterface.h"

DriveInterface::DriveInterface(HardwareSerial *ser)  {
    serial=ser;
}

int DriveInterface::begin()    {
    serial->begin(baudrate, SERIAL_8N1, RX_PIN, TX_PIN);
    serial->setTimeout(timeout);
    return 1;
}

// int DriveInterface::begin() {
//     serial->begin(baudrate);
//     serial->setTimeout(timeout);
//     return 1;
// }

void DriveInterface::setBaudrate(long brate)    {
    baudrate=brate;
}

void DriveInterface::setTimeout(long tm)    {
    timeout=tm;
}

int DriveInterface::fetchData() {
    if(serial->available()>=DATA_PACKET_SIZE)   {
        int tmp[data_size];
        byte high_byte;
        byte low_byte;
        for(int i=0;i<data_size;i++)    {
            high_byte=serial->read();
            low_byte=serial->read();
            tmp[i]=(int16_t)((high_byte<<8)+low_byte);
        }
        alert=tmp[0];
        x_axis=tmp[1];
        y_axis=tmp[2];
        rover_heading=tmp[3];
        total_distance = ((tmp[4]&65535)<<16) + (tmp[5]&65535);
        return 1;
    }
    else    {
        return 0;
    }
}

void DriveInterface::sendUpdates()  {
    send_integer(drive_mode);
    send_integer(direction);
    send_integer(speed);
    send_integer(distance);
    send_integer(target_x);
    send_integer(target_y);
    send_integer(system_time>>16);      // system_time higher 2 bytes
    send_integer(system_time&65535);    // system_time lower 2 bytes
    send_integer(reset);
}

void DriveInterface::writeDriveMode(int dm) {
    drive_mode=dm;
}

void DriveInterface::writeDirection(int dir)    {
    direction=dir;
}

void DriveInterface::writeSpeed(int spd)    {
    speed=spd;
}

void DriveInterface::writeDistance(int dist)    {
    distance=dist;
}

void DriveInterface::writeTargetX(int x)    {
    target_x=x;
}

void DriveInterface::writeTargetY(int y)    {
    target_y=y;
}

void DriveInterface::writeSystemTime(unsigned long time)    {
    system_time=time;
}

void DriveInterface::writeReset(int rst)    {
    reset=rst;
}

int DriveInterface::getAlert() const    {
    return alert;
}

int DriveInterface::getAxisX() const    {
    return x_axis;
}

int DriveInterface::getAxisY() const    {
    return y_axis;
}

int DriveInterface::getRoverHeading() const {
    return rover_heading;
}

unsigned long DriveInterface::getTotalDistance() const   {
    return total_distance;
}

void DriveInterface::send_integer(int d)    {
    serial->write(d>>8);
    serial->write(d&255);
}