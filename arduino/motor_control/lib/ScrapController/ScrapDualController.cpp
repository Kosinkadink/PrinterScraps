#include "ScrapController.h"

//ScrapDualController::

ScrapDualController::ScrapDualController() {
	powerInit1 = powerGLOBALInit;
	powerInit2 = powerGLOBALInit;
}

ScrapDualController::ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2) {
	powerInit1 = powerGLOBALInit;
	powerInit2 = powerGLOBALInit;
	attachMotor1(mot1);
	attachMotor2(mot2);
	attachEncoder1(enc1);
	attachEncoder2(enc2);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	speedControl2 = ScrapMotorControl(*motor2,*encoder2);
	speedControl1.setMinPower(minSlowPower1);
	speedControl2.setMinPower(minSlowPower2);
	stop();
}

ScrapDualController::ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2, ScrapSwitch& swi1, ScrapSwitch& swi2) {
	powerInit1 = powerGLOBALInit;
	powerInit2 = powerGLOBALInit;
	attachMotor1(mot1);
	attachMotor2(mot2);
	attachEncoder1(enc1);
	attachEncoder2(enc2);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	speedControl2 = ScrapMotorControl(*motor2,*encoder2);
	speedControl1.setMinPower(minSlowPower1);
	speedControl2.setMinPower(minSlowPower2);
	attachSwitch1(swi1);
	attachSwitch2(swi2);
	stop();
}

// move back until switches are activated
bool ScrapDualController::performReset() {
	speedControl1.stop();
	speedControl2.stop();
	motor1->setDirection(-1);
	motor2->setDirection(-1);
	// check if both switches are pressed
	if (switch1->getIfPressed() && switch2->getIfPressed()) {
		speedControl1.setSpeed(0);
		speedControl2.setSpeed(0);
		encoder1->resetCount();
		encoder2->resetCount();
		speedControl1.performMovement();
		speedControl2.performMovement();
		return true;
	}
	else {
		if (switch1->getIfPressed()) {
			speedControl1.setSpeed(0);
		}
		else {
			//motor1->setMotor(-225);
			speedControl1.setSpeed(0.0011);
		}
		if (switch2->getIfPressed()) {
			speedControl2.setSpeed(0);
		}
		else {
			//motor2->setMotor(-225);
			speedControl2.setSpeed(0.0011);
		}
		speedControl1.performMovement();
		speedControl2.performMovement();
		return false;
	}
}

bool ScrapDualController::set(int g1, int g2) {
	goal1 = g1;
	goal2 = g2;
	/*// set a smaller slowdown thresh if this is a small movement
	if (abs(goal1-encoder1->getCount()) <= slowdownGLOBALThresh) {
		slowdownThresh1 = shortSlowdownThresh;
	}
	else {
		slowdownThresh1 = slowdownGLOBALThresh;
	}
	if (abs(goal2-encoder2->getCount()) <= slowdownGLOBALThresh) {
		slowdownThresh2 = shortSlowdownThresh;
	}
	else {
		slowdownThresh2 = slowdownGLOBALThresh;
	}*/
	powerInit1 = powerGLOBALInit;
	powerInit2 = powerGLOBALInit;
	return checkIfDone();
}

bool ScrapDualController::set(int goal_both) {
	return set(goal_both,goal_both);
}

bool ScrapDualController::performMovement() {
	//check if already done moving
	if (checkIfDone()) { 
		stop();
		return true;
	}
	//else, gotta do stuff
	if (checkIfDone1()) {
		motor1->stop();
	}
	else {
		if (encoder1->getCount() < goal1) {
			motor1->setMotor(calcPower1());
		}
		else {
			motor1->setMotor(-calcPower1());
		}
	}
	if (checkIfDone2()) {
		motor2->stop();
	}
	else {
		if (encoder2->getCount() < goal2) {
			motor2->setMotor(calcPower2());
		}
		else {
			motor2->setMotor(-calcPower2());
		}
	}
	// based on separation, balance power of each motor
	balancePower();
	
	return false;

}

bool ScrapDualController::performMovementSpeed() {
	//check if already done moving
	if (checkIfDone()) { 
		stop();
		return true;
	}
	//else, gotta do stuff
	if (checkIfDone1()) {
		speedControl1.stop();
	}
	else {
		if (encoder1->getCount() < goal1) {
			speedControl1.setControl(calcSpeed1());
		}
		else {
			speedControl1.setControl(-calcSpeed1());
		}
	}
	if (checkIfDone2()) {
		speedControl2.stop();
	}
	else {
		if (encoder2->getCount() < goal2) {
			speedControl2.setControl(calcSpeed2());
		}
		else {
			speedControl2.setControl(-calcSpeed2());
		}
	}
	// based on separation, balance power of each motor
	balanceSpeed();
	// now execute movement
	speedControl1.performMovement();
	speedControl2.performMovement();
	
	return false;

}


// calculate speed to give motor
float ScrapDualController::calcSpeed1() {
	int diff = getDiff1();
	return speedControl1.convertToSpeed(map(diff,1,slowdownThresh1,minEncSpeed,maxEncSpeed));
}

// calculate speed to give motor
float ScrapDualController::calcSpeed2() {
	int diff = getDiff2();
	return speedControl2.convertToSpeed(map(diff,1,slowdownThresh2,minEncSpeed,maxEncSpeed));
}


// calculate power to give motor
int ScrapDualController::calcPower1() {
	int diff = abs(encoder1->getCount() - goal1);
	if (diff > slowdownThresh1) {
		return powerInit1;
	}
	else {
		return map(diff,0,slowdownThresh1,minSlowPower1,powerInit1);
	}
}

int ScrapDualController::calcPower2() {
	int diff = abs(encoder2->getCount() - goal2);
	if (diff > slowdownThresh2) {
		return powerInit2;
	}
	else {
		return map(diff,0,slowdownThresh2,minSlowPower2,powerInit2);
	}
}

int ScrapDualController::getDiff1() {
	return abs(encoder1->getCount() - goal1);
}

int ScrapDualController::getDiff2() {
	return abs(encoder2->getCount() - goal2);
}

void ScrapDualController::stop() {
	speedControl1.setSpeed(0);
	speedControl2.setSpeed(0);
	speedControl1.performMovement();
	speedControl2.performMovement();
}

// increment or decrement target speed
void ScrapDualController::incrementSpeed(float prop) {
	speedControl1.incrementSpeed(prop);
	speedControl2.incrementSpeed(prop);
}

void ScrapDualController::decrementSpeed(float prop) {
	speedControl1.decrementSpeed(prop);
	speedControl2.decrementSpeed(prop);
}


// increment or decrement target power
void ScrapDualController::incrementPower(int val) {
	powerInit1 = min(255,motor1->getPower()+val);
	powerInit2 = min(255,motor2->getPower()+val);
}

void ScrapDualController::decrementPower(int val) {
	powerInit1 = max(minSlowPower1,motor1->getPower()-val);
	powerInit2 = max(minSlowPower2,motor2->getPower()-val);
}
// set to powerInit
void ScrapDualController::resumePower() {
	//powerInit1 = powerGLOBALInit;
	//powerInit2 = powerGLOBALInit;
}

// redistribute power, based on direction of movement and enc difference
void ScrapDualController::balancePower() {
	// if 1 too far ahead, balance power towards 2
	if ((encoder1->getCount() - encoder2->getCount())*motor1->getDirection() >= diffTolerance) {
		movePowerToward2();
	}
	// if 2 too far ahead, balance power towards 1
	else if ((encoder1->getCount() - encoder2->getCount())*motor1->getDirection() <= -diffTolerance) {
		movePowerToward1();
	}
	// otherwise, resume normal behavior
	else {
		powerInit1 = powerGLOBALInit;
		powerInit2 = powerGLOBALInit;
	}
}

void ScrapDualController::balanceSpeed() {
	float prop = 0.4;
	
	// if 1 too far ahead, balance power towards 2
	if ((encoder1->getCount() - encoder2->getCount())*motor1->getDirection() >= diffTolerance) {
		moveSpeedToward2(prop);
	}
	// if 2 too far ahead, balance power towards 1
	else if ((encoder1->getCount() - encoder2->getCount())*motor1->getDirection() <= -diffTolerance) {
		moveSpeedToward1(prop);
	}
	// otherwise, do not change speed
}

// balance speed
void ScrapDualController::moveSpeedToward1(float prop) {
	speedControl1.incrementSpeed(prop);
	speedControl2.decrementSpeed(prop/2.0);
}

void ScrapDualController::moveSpeedToward2(float prop) {
	speedControl1.decrementSpeed(prop/2.0);
	speedControl2.incrementSpeed(prop);
}

// balance power
void ScrapDualController::movePowerToward1(int val) {
	powerInit1 = min(255,motor1->getPower()+val);
	powerInit2 = max(minSlowPower2,motor2->getPower()-val);
}

void ScrapDualController::movePowerToward2(int val) {
	powerInit1 = max(minSlowPower1,motor1->getPower()-val);
	powerInit2 = min(255,motor2->getPower()+val);
}

// check if encoder count is within tolerance of goal
bool ScrapDualController::checkIfDone() {
	return (checkIfDone1() && checkIfDone2());
}

bool ScrapDualController::checkIfDone1() {
	return (encoder1->getCount() >= goal1 - encTolerance ) && (encoder1->getCount() <= goal1 + encTolerance );
}

bool ScrapDualController::checkIfDone2() {
	return (encoder2->getCount() >= goal2 - encTolerance ) && (encoder2->getCount() <= goal2 + encTolerance );
}

// attach motors + encoders to be used
void ScrapDualController::attachMotor1(ScrapMotor& mot) {
	motor1 = &mot;
}

void ScrapDualController::attachMotor2(ScrapMotor& mot) {
	motor2 = &mot;
}

void ScrapDualController::attachEncoder1(ScrapEncoder& enc) {
	encoder1 = &enc;
}

void ScrapDualController::attachEncoder2(ScrapEncoder& enc) {
	encoder2 = &enc;
}

