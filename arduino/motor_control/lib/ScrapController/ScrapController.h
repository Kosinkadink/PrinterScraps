#ifndef SCRAPCONTROLLER_H
#define SCRAPCONTROLLER_H
#include "Arduino.h"


class ScrapMotor {
	private:
		int PIN_D1;
		int PIN_D2;
		int PIN_PWM;
		int currPower;
		int currDir;
		int powerMultiplier = 1;
		void initMotor();
	public:
		ScrapMotor(int PinD1, int PinD2, int PinPWM, int dirMultip = 1);
		void setMotor(int pwm);
		void setDirection(int pwm);
		void setDirectionMultiplier(int multi);
		void setPower(int pwm);
		void incrementPower() { setPower(currDir+1); };
		void decrementPower() { setPower(currDir-1); };
		int getDirection();
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

class ScrapFullController {
	private:
		ScrapMotor* motorLeftY;
		ScrapMotor* motorRightY;
		ScrapMotor* motorX;
		ScrapEncoder* encLeftY;
		ScrapEncoder* encRightY;
		ScrapEncoder* encX;
	public:
		ScrapFullController();


};


class ScrapController {
	private:
		int goal1;
		int powerGLOBALMax = 255;
		int powerGLOBALMin = 90;
		int powerGLOBALInit = 200;
		int powerMax;
		int powerMin;
		int powerInit;
		int encTolerance = 10;
		ScrapMotor* motor1;
		ScrapEncoder* encoder1;
	public:
		ScrapController();
		ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1);
		bool set(int g1);
		bool performMovement();
		bool incrementPower(int val = 1);
		bool decrementPower(int val = 1);
		bool getCount();
		void attachMotor1(ScrapMotor& mot);
		void attachEncoder1(ScrapEncoder& enc);


};


class ScrapDualController {
	private:
		int goal1;
		int goal2;
		int powerGLOBALMax = 255;
		int powerGLOBALMin = 90;
		int powerGLOBALInit = 200;
		int powerMax;
		int powerMin;
		int powerInit;
		int encTolerance = 5;
		ScrapMotor* motor1;
		ScrapMotor* motor2;
		ScrapEncoder* encoder1;
		ScrapEncoder* encoder2;
	public:
		ScrapDualController();
		ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2);
		bool set(int g1,int g2); //returns state of 'done'
		bool checkIfDone();
		bool checkIfDone1();
		bool checkIfDone2();
		int calcPower1();
		int calcPower2();
		bool performMovement();
		bool incrementPower(int val = 1);
		bool decrementPower(int val = 1);
		void stop();
		int getCount1() { return encoder1->getCount(); };
		int getCount2() { return encoder2->getCount(); };
		int getCount() { return (getCount1()+getCount2())/2; }; //returns average of encoder counts
		void attachMotor1(ScrapMotor& mot);
		void attachMotor2(ScrapMotor& mot);
		void attachEncoder1(ScrapEncoder& enc);
		void attachEncoder2(ScrapEncoder& enc);

};



#endif
