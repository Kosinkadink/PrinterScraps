#ifndef SCRAPCONTROLLER_H
#define SCRAPCONTROLLER_H
#include "Arduino.h"


class ScrapController {

};



class ScrapMotor {
	private:
		int PIN_D1;
		int PIN_D2;
		int PIN_PWM;
		int currPower;
		int currDir;
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
		int PINA_INTERRUPT;
		int PINB_CHECKER;
		void initEncoder();
	public:
		ScrapEncoder(int pinA, int pinB);
		int getCount();
		void resetCount();
		void incrementCount();
		void decrementCount();
		void checkEncoder(); //deprecated :(
};


#endif
