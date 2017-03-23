#ifndef SCRAPCONTROLLER_H
#define SCRAPCONTROLLER_H
#include "Arduino.h"


class ScrapController {
	private:

	public:

};



class ScrapMotor {
	private:
		const int PIN_D1;
		const int PIN_D2;
		const int PIN_PWM;
		const int currPower;
		const int currDir;
		void initMotor();
	public:
		ScrapMotor(int PinD1, int PinD2, int PinPWM);
		void setDirection(int pwm);
		int getDirection();
		void setPower(int pwm);
		int getPower();
		void stop();
};

class ScrapEncoder {
	private:
		volatile int encCount;
		const int PINA_INTERRUPT;
		const int PINB;
		void initEncoder();
	public:
		ScrapEncoder(int PinA, int pinB);
		int getCount();
		int resetCount();
		int checkEncoder();
};


#endif