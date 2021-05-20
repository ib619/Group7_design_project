#ifndef DRIVE_H
#define DRIVE_H

#include <Arduino.h>

class Motor {
public:
	Motor();
	void init(int dirPin, int sPin);
	void setDirection(bool dir);
	virtual void move(int speed) = 0;

	int directionPin;
	int pwmPin;
	bool direction; //HIGH for forward, LOW for backwards
};

class LeftMotor : public Motor {
public:
	void move(int speed);
};

class RightMotor : public Motor {
public:
	void move(int speed);

};

class Rover {
public:
	Rover();
	void init(LeftMotor *left, RightMotor *right);
	void setDirection(bool dir);
	void move(float travelDist);
	void decodeCommand(int dist, int spd, int dir);
	bool command_running = 0;

private:
	LeftMotor *LM;
	RightMotor *RM;
	int distance_setpoint = 0;
	int speed_setpoint = 0;
	int direction_setpoint = 0;

	bool turnCW_flag = 0;
	bool turnACW_flag = 0;
	bool move_flag = 0;

	void stop();
};

#endif