#ifndef CONTROL_INTERFACE_API_H
#define CONTROL_INTERFACE_API_H

#include <Arduino.h>

#define CONTROL_PACKET_SIZE 18

class ControlInterface {
    public:
        ControlInterface(HardwareSerial *ser);
        int begin();
        int fetchData();   // returns 0 if no updates, 1 if there is new data available
        void sendUpdates(); // send data values
        void setBaudrate(long brate); // default baudrate 38400
        void setTimeout(long tm);
        void flushReadBuffer();

        int getDriveMode() const;
        int getDirection() const;
        int getSpeed() const;
        int getDistance() const;
        int getTargetX() const;
        int getTargetY() const;
        unsigned long getSystemTime() const;    // 4 bytes
        int getReset();

        void writeBatterylevel(int batt);
        void writeRange(int rng);
        void writeObstacle(int obs);
        void writeAlert(int alrt);
        void writeAxisX(int x);
        void writeAxisY(int y);
        void writeRoverHeading(int heading);
        void writeBatterySOH(int soh);
        void writeBatteryState(int state);
        void writeTotalDistance(unsigned long dist);
    
    private:
        void send_integer(int data);

        HardwareSerial *serial;
        long baudrate=38400;
        long serial_timeout=10;  // serial port timeout in milliseconds

        int control_packet_size=CONTROL_PACKET_SIZE;  // multiple of 2 as all data comes in int
        int ctrl_size=CONTROL_PACKET_SIZE/2;

        // control values
        int drive_mode=0;
        int direction=0;
        int speed=0;
        int distance=0;
        int target_x=0;
        int target_y=0;
        unsigned long system_time=0;    // 4 bytes
        int reset=0;

        // data values
        int battery_level=0;
        int rover_range=0;
        int obstacle_detected=0;
        int alert=0;
        int x_axis=0;
        int y_axis=0;
        int rover_heading=0;
        int battery_SOH=0;
        int battery_state=0;
        unsigned long total_distance=0;
};

#endif