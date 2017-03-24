#include "ScrapController.h"


ScrapMotor::ScrapMotor(int PinD1, int PinD2, int PinPWM) {
	PIN_D1 = PinD1;
	PIN_D2 = PinD2;
	PIN_PWM = PinPWM;
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

// set direction based on sign of number
void ScrapMotor::setDirection(int pwm) {
	if (pwm > 0) {
		digitalWrite(PIN_D1,HIGH);
		digitalWrite(PIN_D2,LOW);
		currDir = 1;
	}
	else if (pwm < 0) {
		digitalWrite(PIN_D1,HIGH);
		digitalWrite(PIN_D2,LOW);
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

// set power (PWM)
void ScrapMotor::setPower(int pwm) {
	analogWrite(PIN_PWM,pwm);
	currPower = pwm;
}

// return current power (PWM)
int ScrapMotor::getPower() {
	return currPower;
}

// stop the motor
void ScrapMotor::stop() {
	setDirection(0);
}