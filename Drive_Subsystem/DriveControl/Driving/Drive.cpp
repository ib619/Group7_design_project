#include "Drive.h"

Motor::Motor() {}

void Motor::init(int dirPin, int sPin) {
	directionPin = dirPin;
	pwmPin = sPin;
	pinMode(directionPin, OUTPUT);
	pinMode(pwmPin, OUTPUT);
}

void Motor::setDirection(bool dir) {
	direction = dir;
}

void LeftMotor::move(int speed) {
	digitalWrite(directionPin, !direction);
	analogWrite(pwmPin, speed);
}

void RightMotor::move(int speed) {
	digitalWrite(directionPin, direction);
	analogWrite(pwmPin, speed);
}


Rover::Rover() {};

void Rover::init(LeftMotor *left, RightMotor *right) {
	LM = left;
	RM = right;
}

void Rover::setDirection(bool dir) {
	LM->setDirection(dir);
	RM->setDirection(dir);
}

void Rover::moveForward(float travelDist) {
	this->setDirection(HIGH);
	if (move_flag) {
		if (distance_setpoint > 0) {
			LM->move(speed_setpoint);
			RM->move(speed_setpoint);
			distance_setpoint = distance_setpoint - travelDist;
			command_running = 1;
		}
		else {
			move_flag = 0;
			command_running = 0;
			stop();
		}
	}
}

void Rover::turnCW(float angle) {
	if (turnCW_flag) {
		LM->setDirection(HIGH);
		RM->setDirection(HIGH);
		if (direction_setpoint > 0) {
			LM->move(speed_setpoint);
			RM->move(50);
			direction_setpoint = direction_setpoint - angle;
			command_running = 1;
		}
		else {
			turnCW_flag = 0;
			command_running = 0;
			stop();
		}
	}
}

void Rover::turnACW(float angle) {
	if (turnACW_flag) {
		LM->setDirection(HIGH);
		RM->setDirection(HIGH);
		if (direction_setpoint > 0) {
			LM->move(50);
			RM->move(speed_setpoint);
			direction_setpoint = direction_setpoint - angle;
			command_running = 1;
		}
		else {
			turnACW_flag = 0;
			command_running = 0;
			stop();
		}
	}
}

void Rover::decodeCommand(int dist, int spd, int dir) {
	distance_setpoint = dist;
	speed_setpoint = spd;
	direction_setpoint = abs(dir);

	if (dir > 0) { turnACW_flag = 1; turnCW_flag = 0; move_flag = 0;}
	if (dir < 0) { turnACW_flag = 0; turnCW_flag = 1; move_flag = 0;}
	if (dir == 0) { turnACW_flag = 0; turnCW_flag = 0; move_flag = 1;}
}

void Rover::stop() {
	analogWrite(LM->pwmPin, 0);
	analogWrite(RM->pwmPin, 0);
}