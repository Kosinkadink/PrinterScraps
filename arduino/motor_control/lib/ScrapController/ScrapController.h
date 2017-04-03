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
		void checkEncoder();
		void checkEncoderFlipped();
};


class ScrapSwitch {
	private:
		int PIN_SWITCH;
		bool highWhenOpen = true;
		void initSwitch() { pinMode(PIN_SWITCH,INPUT); };
	public:
		ScrapSwitch(int pinSwitch) { PIN_SWITCH=pinSwitch; initSwitch(); };
		void setOpenHigh(bool openHigh) { highWhenOpen = openHigh; };
		bool getIfPressed() {
			if (highWhenOpen) 
				return (digitalRead(PIN_SWITCH)==LOW);
			else
				return (digitalRead(PIN_SWITCH)==HIGH);
		};
};


class ScrapController {
	private:
		int goal1;
		int powerGLOBALInit = 200; // default starting speed
		int slowdownGLOBALThresh = 250; // default slowdown enc diff
		int powerInit;
		int encTolerance = 5; // +/- range around goal
		int slowdownThresh = 250; // slow down range
		int shortSlowdownThresh = 50; // used for short distances
		int minSlowPower = 120; // minimum power
		int minDecrementPower = 130; // minimum power of decrementPower
		ScrapMotor* motor1;
		ScrapEncoder* encoder1;
		ScrapSwitch* switch1;
	public:
		ScrapController();
		ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1);
		ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1, ScrapSwitch& swi1);
		bool set(int g1);
		int getGoal1() { return goal1; };
		int getGoal() { return getGoal1(); };
		bool checkIfDone1();
		bool checkIfDone() { return checkIfDone1(); };
		int calcPower1();
		int calcPower() { return calcPower1(); };
		bool performMovement();
		bool performReset();
		void incrementPower(int val = 1);
		void decrementPower(int val = 1);
		void stop() { motor1->stop(); };
		int getCount1() { return encoder1->getCount(); };
		int getCount() { return getCount1(); };
		void attachMotor1(ScrapMotor& mot);
		void attachEncoder1(ScrapEncoder& enc);
		void attachSwitch1(ScrapSwitch& swi) { switch1 = &swi; };
};


class ScrapDualController {
	private:
		int goal1;
		int goal2;
		int powerGLOBALInit = 255; // default starting speed
		int slowdownGLOBALThresh = 300; // default slowdown enc diff
		int powerInit1;
		int powerInit2;
		int diffTolerance = 25; //max diff in encoder values
		int encTolerance = 5; // max window of error from set goal
		int slowdownThresh = 300; // slow down range
		int shortSlowdownThresh = 75; // used for short distances
		int minSlowPower1 = 190; // minimum power of motor1
		int minSlowPower2 = 175; // minimum power of motor2
		ScrapMotor* motor1;
		ScrapMotor* motor2;
		ScrapEncoder* encoder1;
		ScrapEncoder* encoder2;
		ScrapSwitch* switch1;
		ScrapSwitch* switch2;
	public:
		ScrapDualController();
		ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2);
		ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2, ScrapSwitch& swi1, ScrapSwitch& swi2);
		bool set(int g1,int g2); //returns state of 'done'
		bool set(int goal_both); //returns state of 'done'
		int getGoal1() { return goal1; };
		int getGoal2() { return goal2; };
		int getGoal() { return (goal1+goal2)/2; };
		bool checkIfDone();
		bool checkIfDone1();
		bool checkIfDone2();
		int calcPower1();
		int calcPower2();
		bool performMovement();
		bool performReset();
		void incrementPower(int val = 1);
		void decrementPower(int val = 1);
		void movePowerToward1(int val = 1);
		void movePowerToward2(int val = 1);
		void balancePower();
		void stop();
		int getCount1() { return encoder1->getCount(); };
		int getCount2() { return encoder2->getCount(); };
		int getCount() { return (getCount1()+getCount2())/2; }; //returns average of encoder counts
		void attachMotor1(ScrapMotor& mot);
		void attachMotor2(ScrapMotor& mot);
		void attachEncoder1(ScrapEncoder& enc);
		void attachEncoder2(ScrapEncoder& enc);
		void attachSwitch1(ScrapSwitch& swi) { switch1 = &swi; };
		void attachSwitch2(ScrapSwitch& swi) { switch2 = &swi; };
};


class ScrapFullController {
	private:
		ScrapController* xControl;
		ScrapDualController* yControl;
		float diffDecim = 0.02; // percentage diff from desired proportion
		float desiredProportion; // x_goal/y_goal proportion
	public:
		ScrapFullController();
		ScrapFullController(ScrapController& xCont, ScrapDualController& yCont);
		bool set(int gx, int gy);
		int getGoalX() { return xControl->getGoal(); };
		int getGoalY() { return yControl->getGoal(); };
		int getCountX() { return xControl->getCount(); };
		int getCountY() { return yControl->getCount(); };
		bool checkIfDoneX() { return xControl->checkIfDone(); };
		bool checkIfDoneY() { return yControl->checkIfDone(); };
		bool checkIfDone() { return checkIfDoneX() && checkIfDoneY(); };
		bool performMovement();
		bool performReset();
		float getMovementProportion(); 
		void balancePower();
		void movePowerTowardX(int val = 1);
		void movePowerTowardY(int val = 1);
		void stop() { xControl->stop(); yControl->stop(); };
		void attachControllerX(ScrapController& xCont);
		void attachControllerY(ScrapDualController& yCont);
};


#endif
