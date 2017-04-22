#ifndef SCRAPCONTROLLER_H
#define SCRAPCONTROLLER_H
#include "Arduino.h"


// classes
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
		void incrementPower(int val = 1) { setPower(currDir+val); };
		void decrementPower(int val = 1) { setPower(currDir-val); };
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


class ScrapMotorControl {
	private:
		unsigned long prevTime = 0;
		float prevSpeed = 0.0;
		float speedGoal = 0.0;
		int prevCount = 0;
		int minPower = 50;
		float minSpeed;
		float maxSpeed;
		ScrapMotor* motor;
		ScrapEncoder* encoder;
		float calcSpeed(); // calculates speed and updates relevant vals
	public:
		ScrapMotorControl();
		ScrapMotorControl(ScrapMotor& mot, ScrapEncoder& enc);
		void setControl(float newSpeed); // set direction + speed
		void setSpeed(float newSpeed); // set direction only
		void setMinSpeed(float newMin) { minSpeed = newMin; };
		void setMaxSpeed(float newMax) { maxSpeed = newMax; };
		void incrementSpeed(int speedEncDiff);
		void decrementSpeed(int speedEncDiff);
		float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
		float constrainFloat(float x, float min, float max);
		float convertToSpeed(int encPerSec);
		void setMinPower(int power) { minPower = power; };
		void reset();
		void stop();
		int getCount() { return encoder->getCount(); };
		float getSpeed(); // returns speed
		float getSpeedGoal() { return speedGoal; }; // return speed goal
		unsigned long getTime();
		void performMovement();
		void attachMotor(ScrapMotor& mot) { motor = &mot; };
		void attachEncoder(ScrapEncoder& enc) { encoder = &enc; };
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
		int encTolerance = 5; // +/- range around goal
		int slowdownThresh = 200; // slow down range
		int minSlowPower = 120; // minimum power
		int minEncSpeed = 200;
		int maxEncSpeed = 1000;
		ScrapMotor* motor1;
		ScrapEncoder* encoder1;
		ScrapSwitch* switch1;
		ScrapMotorControl speedControl1;
	public:
		ScrapController();
		ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1);
		ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1, ScrapSwitch& swi1);
		bool set(int g1);
		int getGoal1() { return goal1; };
		int getGoal() { return getGoal1(); };
		int getDiff1();
		bool checkIfDone1();
		bool checkIfDone() { return checkIfDone1(); };
		float calcSpeed1();
		float calcSpeed() { return calcSpeed1(); };
		bool performMovement();
		bool performReset();
		void incrementSpeed(int speedEncDiff);
		void decrementSpeed(int speedEncDiff);
		void stop();
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
		int diffTolerance = 50; //max diff in encoder values
		int encTolerance = 5; // max window of error from set goal
		int slowdownThresh1 = 200; // slow down range
		int slowdownThresh2 = 200; // slow down range
		int minSlowPower1 = 197; // minimum power of motor1
		int minSlowPower2 = 180; // minimum power of motor2
		int minEncSpeed = 200;
		int maxEncSpeed = 1000;
		int encSpeedBalance = 10;
		ScrapMotor* motor1;
		ScrapMotor* motor2;
		ScrapEncoder* encoder1;
		ScrapEncoder* encoder2;
		ScrapSwitch* switch1;
		ScrapSwitch* switch2;
		ScrapMotorControl speedControl1;
		ScrapMotorControl speedControl2;
	public:
		ScrapDualController();
		ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2);
		ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2, ScrapSwitch& swi1, ScrapSwitch& swi2);
		bool set(int g1,int g2); //returns state of 'done'
		bool set(int goal_both); //returns state of 'done'
		int getGoal1() { return goal1; };
		int getGoal2() { return goal2; };
		int getGoal() { return (goal1+goal2)/2; };
		int getDiff1();
		int getDiff2();
		bool checkIfDone();
		bool checkIfDone1();
		bool checkIfDone2();
		float calcSpeed1();
		float calcSpeed2(); 
		bool performMovement();
		bool performReset();
		void incrementSpeed(int speedEncDiff);
		void decrementSpeed(int speedEncDiff);
		void moveSpeedToward1(int speedEncDiff);
		void moveSpeedToward2(int speedEncDiff);
		void balanceSpeed();
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
		float diffDecim = 0.01; // percentage diff from desired proportion
		float desiredProportion; // x_goal/y_goal proportion
		int encSpeedBalance = 30;
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
		void balanceSpeed(); 
		void moveSpeedTowardX(int speedEncDiff);
		void moveSpeedTowardY(int speedEncDiff);
		void stop() { xControl->stop(); yControl->stop(); };
		void attachControllerX(ScrapController& xCont);
		void attachControllerY(ScrapDualController& yCont);
};


#endif
