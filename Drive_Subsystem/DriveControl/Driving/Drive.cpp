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
		if (lock_direction) {
			lockX = myDirectionX;
			lockY = myDirectionY;
			lock_direction = 0;
		}

		float crossprod = myDirectionX * lockY - lockX * myDirectionY;
		float dotprod = myDirectionX * lockX + myDirectionY * lockY;

		float lockAbs = sqrt(pow(lockX, 2) + pow(lockY, 2));
		float myDirAbs = sqrt(pow(myDirectionX, 2) + pow(myDirectionY, 2));

		float cosine = dotprod / (lockAbs * myDirAbs);

		float angle_rad = acos(cosine);
		float angle = angle_rad * (180 / 3.14159);

		if (crossprod >= 0) { Error = angle; }
		if (crossprod < 0) { Error = -angle; }

		cumError = cumError + Error * timeDiff;
		rateError = (Error - lastError) / timeDiff;

		int control = Kp * Error + Ki * cumError + Kd * rateError;

		if (enablePID) {
			correction = saturation(control, speed_setpoint);
		}
		else { correction = 0; }
		
		if (distance_setpoint > 0) {
			LM->move(speed_setpoint + correction);
			RM->move(speed_setpoint - correction);
			distance_setpoint = distance_setpoint - travelDist;
			lastError = Error;
		}
		else {
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
		}
		else {
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
			stop();
		}
	}
}

void Rover::decodeCommand(int dm, int dist, int spd, int dir) {
	drive_mode = dm;
	speed_setpoint = spd;
	lock_direction = 1;
	Error = 0;
	lastError = 0;
	cumError = 0;
	rateError = 0;

	if (drive_mode == 1 || drive_mode == 0) {
		distance_setpoint = abs(dist);
		direction_setpoint = abs(dir);
		command_running = 1;

		if (dir > 0 && dist > 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 1; moveBack_flag = 0; }
		if (dir > 0 && dist < 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 0; moveBack_flag = 1; }
		if (dir < 0 && dist > 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 1; moveBack_flag = 0; }
		if (dir < 0 && dist < 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 0; moveBack_flag = 1; }
		if (dir == 0 && dist > 0) { turnACW_flag = 0; turnCW_flag = 0; moveForward_flag = 1; moveBack_flag = 0; }
		if (dir == 0 && dist < 0) { turnACW_flag = 0; turnCW_flag = 0; moveForward_flag = 0; moveBack_flag = 1; }
		if (dir > 0 && dist == 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 0; moveBack_flag = 0; }
		if (dir < 0 && dist == 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 0; moveBack_flag = 0; }
	}
}

void Rover::decodeT2C(int target_x, int target_y, float myPosX, float myPosY, float myDirX, float myDirY) {


	if (drive_mode == 2) {
		//find vector MT
		float mtX = target_x - myPosX; 
		float mtY = target_y - myPosY;

		float crossprod = myDirX * mtY - mtX * myDirY;
		float dotprod = myDirX * mtX + myDirY * mtY;

		float mtAbs = sqrt(pow(mtX, 2) + pow(mtY, 2));
		float myDirAbs = sqrt(pow(myDirX, 2) + pow(myDirY, 2));

		float cosine = dotprod / (mtAbs * myDirAbs);

		float angle_rad = acos(cosine);
		float angle = angle_rad * (180 / 3.14159);

		if (crossprod > 0) { turnACW_flag = 1; turnCW_flag = 0; moveForward_flag = 1; moveBack_flag = 0; }
		if (crossprod < 0) { turnACW_flag = 0; turnCW_flag = 1; moveForward_flag = 1; moveBack_flag = 0; }
		if (crossprod == 0) { turnACW_flag = 0; turnCW_flag = 0; moveForward_flag = 1; moveBack_flag = 0; }

		distance_setpoint = mtAbs;
		direction_setpoint = angle;
		command_running = 1;
	}
}

void Rover::stop() {
	analogWrite(LM->pwmPin, 0);
	analogWrite(RM->pwmPin, 0);
}

void Rover::action(float travelDist, float angle) {
	if (drive_mode == 0) {
		stop();
		command_running = 0;
	}
	else if (drive_mode == 1 || drive_mode == 2) {
		if (direction_setpoint > 0) {
			turnCW(angle);
			turnACW(angle);
		}
		else {
			turnCW_flag = 0; turnACW_flag = 0;
			moveForward(travelDist);
			moveBack(travelDist);
			if (distance_setpoint <= 0) { command_running = 0; }
		}
	}
}

void Rover::sampleTime(unsigned long sysTime) {
	lastTime = curTime;
	curTime = sysTime;
	timeDiff = curTime - lastTime;
}

void Rover::sampleDirection(float myDirX, float myDirY) {
	myDirectionX = myDirX;
	myDirectionY = myDirY;
}

int Rover::saturation(int input, int spd) {
	if (input > 255 - spd) { return 255 - spd; }
	else if (input < -spd) { return -spd; }
	else { return input; }
}