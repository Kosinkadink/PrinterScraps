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

// map function for floats
float ScrapMotorControl::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
	return (constrainFloat(x,in_min,in_max)-in_min)*(out_max-out_min)/(in_max-in_min) + out_min;
}

// constrain function for floats
float ScrapMotorControl::constrainFloat(float x, float min, float max) {
	if (x < min)
		return min;
	else if (x > max)
		return max;
	else
		return x;
}

// convert encoder values per second into values per microsecond
float ScrapMotorControl::convertToSpeed(int encPerSec) {
	// million microseconds in one second
	return ((float)encPerSec)/1000000.0;
}

void ScrapMotorControl::reset() {
	speedGoal = 0;
	motor->stop();
	prevSpeed = 0;
	prevCount = 0;
	prevTime = 0;
}

void ScrapMotorControl::stop() {
	speedGoal = 0;
}

void ScrapMotorControl::setSpeed(float newSpeed) {
	speedGoal = newSpeed;
}

void ScrapMotorControl::setControl(float newSpeed) {
	if (newSpeed < 0) {
		motor->setDirection(-1);
		setSpeed(newSpeed*-1);
	}
	else {
		motor->setDirection(1);
		setSpeed(newSpeed);
	}
}

void ScrapMotorControl::performMovement() {
	if (speedGoal == 0) {
		reset();
	}
	else {
		// calculate speed
		float currSpeed = calcSpeed();
		int powChange = 0;
		// change power according to proportion of speeds
		if (currSpeed < speedGoal) {
			powChange = (int)mapFloat(speedGoal/currSpeed,1.0,2.0,1.0,6.0);
			motor->setPower(max(minPower,motor->getPower()+powChange));
		}
		else if (currSpeed > speedGoal) {
			powChange = (int)mapFloat(currSpeed/speedGoal,1.0,2.0,1.0,6.0);
			motor->setPower(max(minPower,motor->getPower()-powChange));
		}
	}
}

// change speed by a speed diff not to exceed limits
void ScrapMotorControl::incrementSpeed(int speedEncDiff) {
	float speedDiff = convertToSpeed(speedEncDiff);
	speedGoal = min(maxSpeed,speedGoal+speedDiff);
}

void ScrapMotorControl::decrementSpeed(int speedEncDiff) {
	float speedDiff = convertToSpeed(speedEncDiff);
	speedGoal = max(minSpeed,speedGoal-speedDiff);
}
