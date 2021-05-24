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
	if (moveForward_flag) {
		this->setDirection(HIGH);
		if (distance_setpoint > 0) {
			LM->move(speed_setpoint);
			RM->move(speed_setpoint);
			distance_setpoint = distance_setpoint - travelDist;
			command_running = 1;
		}
		else {
			moveForward_flag = 0;
			command_running = 0;
			stop();
		}
	}
}

void Rover::moveBack(float travelDist) {
	if (moveBack_flag) {
		this->setDirection(LOW);
		if (distance_setpoint > 0) {
			LM->move(speed_setpoint);
			RM->move(speed_setpoint);
			distance_setpoint = distance_setpoint - travelDist;
			command_running = 1;
		}
		else {
			moveBack_flag = 0;
			command_running = 0;
			stop();
		}
	}
}

void Rover::turnCW(float angle) {
	if (turnCW_flag) {
		LM->setDirection(HIGH);
		RM->setDirection(LOW);
		if (direction_setpoint >= 0) {
			LM->move(speed_setpoint);
			RM->move(speed_setpoint);
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
		LM->setDirection(LOW);
		RM->setDirection(HIGH);
		if (direction_setpoint >= 0) {
			LM->move(speed_setpoint);
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

void Rover::decodeCommand(int dm, int dist, int spd, int dir, int targetX, int targetY, float myDirX, float myDirY, int myPosX, int myPosY) {
	drive_mode = dm;

	if (drive_mode == 0 || drive_mode == 1) {
		distance_setpoint = abs(dist);
		speed_setpoint = spd;
		direction_setpoint = abs(dir);

		if (dir > 0 && dist > 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 1; moveBack_flag = 0; }
		if (dir > 0 && dist < 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 0; moveBack_flag = 1; }
		if (dir < 0 && dist > 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 1; moveBack_flag = 0; }
		if (dir < 0 && dist < 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 0; moveBack_flag = 1; }
		if (dir == 0 && dist > 0) { turnACW_flag = 0; turnCW_flag = 0; moveForward_flag = 1; moveBack_flag = 0; }
		if (dir == 0 && dist < 0) { turnACW_flag = 0; turnCW_flag = 0; moveForward_flag = 0; moveBack_flag = 1; }
		if (dir > 0 && dist == 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 0; moveBack_flag = 0; }
		if (dir < 0 && dist == 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 0; moveBack_flag = 0; }
	}
	else if (drive_mode == 2) {
		
		//calculate the coordinate of M
		float mX = myPosX + radius * myDirX; float mY = myPosY + radius * myDirX;

		//calculate vector MT
		float mtX = targetX - mX; float mtY = targetY - mY;

		//dotproduct of current direction with MT
		float dotprod = mtX * myDirX + mtY * myDirY;
		float absMyDir = sqrt(pow(myDirX,2)+pow(myDirY,2));
		float absNewDir = sqrt(pow(mtX, 2) + pow(mtY, 2));

		//find the turn angle
		float cosine = dotprod/ (absMyDir * absNewDir);
		float angle = acos(cosine);

		//find the required travel distance
		float travelDistance = radius + absNewDir;

		//determine the required rotation direction
		float crossprod = myDirX * mtY - mtX * myDirY;

		if (crossprod > 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 1; moveBack_flag = 0; }
		if (crossprod < 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 1; moveBack_flag = 0; }

		distance_setpoint = travelDistance;
		speed_setpoint = spd;
		direction_setpoint = angle * (180 / 3.14159);

	}
}

void Rover::stop() {
	analogWrite(LM->pwmPin, 0);
	analogWrite(RM->pwmPin, 0);
}

void Rover::action(float travelDist, float angle) {
	if (drive_mode == 0) {
		stop();
	}
	else if (drive_mode == 1) {
		if (direction_setpoint > 0) {
			turnCW(angle);
			turnACW(angle);
		}
		else {
			moveForward(travelDist);
			moveBack(travelDist);
		}
	}
	else if (drive_mode == 2) {
		if (direction_setpoint > 0) {
			turnCW(angle);
			turnACW(angle);
		}
		else {
			moveForward(travelDist);
		}
	}
}