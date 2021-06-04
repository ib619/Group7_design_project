#include "DriveInterface.h"

DriveInterface::DriveInterface(HardwareSerial *ser)  {
    serial=ser;
}

int DriveInterface::begin() {
    serial->begin(baudrate);
    serial->setTimeout(timeout);
    return 1;
}

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
        battery_level=tmp[0];
        rover_range=tmp[1];
        obstacle_detected=tmp[2];
        alert=tmp[3];
        x_axis=tmp[4];
        y_axis=tmp[5];
        rover_heading=tmp[6];
        battery_SOH=tmp[7];
        battery_state=tmp[8];
        total_distance = ((tmp[9]&65535)<<16) + (tmp[10]&65535);
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

int DriveInterface::getBatteryLevel() const {
    return battery_level;
}

int DriveInterface::getRange() const    {
    return rover_range;
}

int DriveInterface::getObstacle() const {
    return obstacle_detected;
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

int DriveInterface::getBatterySOH() const   {
    return battery_SOH;
}

int DriveInterface::getBatteryState() const {
    return battery_state;
}

unsigned long DriveInterface::getTotalDistance() const   {
    return total_distance;
}

void DriveInterface::send_integer(int d)    {
    serial->write(d>>8);
    serial->write(d&255);
}