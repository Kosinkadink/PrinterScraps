#include "ScrapController.h"

ScrapMotorControl::ScrapMotorControl() {
	
}


ScrapMotorControl::ScrapMotorControl(ScrapMotor& mot, ScrapEncoder& enc) {
	attachMotor(mot);
	attachEncoder(enc);
}

float ScrapMotorControl::getSpeed() {
	return prevSpeed;
}

float ScrapMotorControl::calcSpeed() {
	// get some values for calculation
	unsigned long newTime = micros();
	int newCount = encoder->getCount();
	float currSpeed = 0;
	// check if prevTime exists
	if (prevTime == 0)
		prevTime = newTime;
		
	unsigned long timeDelta = newTime - prevTime;
	// avoid zero division; no time has passed anyway
	if (timeDelta != 0) {
		currSpeed = float(abs(newCount-prevCount))/float(timeDelta);
	}
	else {
		currSpeed = prevSpeed;
	}
	// update prev values for next iteration
	prevTime = newTime;
	prevCount = newCount;
	prevSpeed = currSpeed;
	
	return currSpeed;
}

void ScrapMotorControl::reset() {
	speedGoal = 0;
	motor->stop();
	prevSpeed = 0;
	prevCount = 0;
	prevTime = 0;
}

void ScrapMotorControl::setSpeed(float newSpeed) {
	speedGoal = newSpeed;
}


void ScrapMotorControl::performMovement() {
	if (speedGoal == 0) {
		reset();
	}
	else {
		float currSpeed = calcSpeed();
		if (currSpeed < speedGoal) {
			motor->setPower(motor->getPower()+1);
		}
		else if (currSpeed > speedGoal) {
			motor->setPower(motor->getPower()-1);
		}
	}
}
