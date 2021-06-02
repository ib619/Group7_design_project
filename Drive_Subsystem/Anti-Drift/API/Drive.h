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
	void decodeCommand(int dm, int dist, int spd, int dir);
	void decodeT2C(int target_x, int target_y, float myPosX, float myPosY, float myDirX, float myDirY);
	void action(float travelDist, float angle);
	void sampleTime(unsigned long sysTime);
	void sampleDirection(float myDirX, float myDirY);
	bool command_running = 0;
	bool turnCW_flag = 0;
	bool turnACW_flag = 0;
	bool moveForward_flag = 0;
	bool moveBack_flag = 0;

private:
	RightMotor *RM;
	LeftMotor* LM;
	int radius = 168;
	int drive_mode = 0;
	float distance_setpoint = 0;
	int speed_setpoint = 0;
	float direction_setpoint = 0;
	int x_setpoint = 0;
	int y_setpoint = 0;

	bool lock_direction = 0;
	float lockX;
	float lockY;
	float myDirectionX;
	float myDirectionY;
	unsigned long curTime = 0;
	unsigned long lastTime = 0;
	unsigned long timeDiff = 0;
	float Error = 0;
	float lastError = 0;
	float cumError = 0;
	float rateError = 0;

	float Kp = 0.6;
	float Ki = 0.0001;
	float Kd = 0;

	void stop();
	void setDirection(bool dir);
	void moveForward(float travelDist);
	void moveBack(float travelDist);
	void turnCW(float angle);
	void turnACW(float angle);
	int saturation(int input, int spd);
};

#endif