#include "ScrapController.h"


ScrapMotor::ScrapMotor(int PinD1, int PinD2, int PinPWM, int dirMultip) {
	PIN_D1 = PinD1;
	PIN_D2 = PinD2;
	PIN_PWM = PinPWM;
	setDirectionMultiplier(dirMultip);
	initMotor();
}

// init motor pins and make sure they are not moving
void ScrapMotor::initMotor() {
	pinMode(PIN_D1,OUTPUT);
	pinMode(PIN_D2,OUTPUT);
	pinMode(PIN_PWM,OUTPUT);
	setDirection(0);
	setPower(0);
}

// set corresponding direction + power
void ScrapMotor::setMotor(int pwm) {
	setDirection(pwm);
	setPower(abs(pwm));
}

// set direction multiplier (lets you flip direction)
void ScrapMotor::setDirectionMultiplier(int multi) {
	if (multi > 0) {
		powerMultiplier = 1; 
	}
	else if (multi < 0) {
		powerMultiplier = -1;
	}
	// do nothing is zero
}

// set direction based on sign of number
void ScrapMotor::setDirection(int pwm) {
	pwm *= powerMultiplier;
	if (pwm > 0) {
		digitalWrite(PIN_D1,HIGH);
		digitalWrite(PIN_D2,LOW);
		currDir = 1;
	}
	else if (pwm < 0) {
		digitalWrite(PIN_D1,LOW);
		digitalWrite(PIN_D2,HIGH);
		currDir = -1;
	}
	else {
		digitalWrite(PIN_D1,LOW);
		digitalWrite(PIN_D2,LOW);
		currDir = 0;
	}
}

// return direction: 1, 0, or -1
int ScrapMotor::getDirection() {
	return currDir;
}

// set power (PWM) - make sure is in valid range
void ScrapMotor::setPower(int pwm) {
	pwm = max(0,min(255,pwm));
	analogWrite(PIN_PWM,pwm);
	currPower = pwm;
}

// return current power (PWM)
int ScrapMotor::getPower() {
	return currPower;
}

// stop the motor
void ScrapMotor::stop() {
	setMotor(0);
}
