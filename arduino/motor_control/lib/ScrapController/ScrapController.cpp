#include "ScrapController.h"

//ScrapController::

ScrapController::ScrapController() {
	powerInit = powerGLOBALInit;
}

ScrapController::ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1) {
	powerInit = powerGLOBALInit;
	attachMotor1(mot1);
	attachEncoder1(enc1);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	stop();
}

ScrapController::ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1, ScrapSwitch& swi1) {
	powerInit = powerGLOBALInit;
	attachMotor1(mot1);
	attachEncoder1(enc1);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	attachSwitch1(swi1);
	stop();
}

// move back until switches are activated
bool ScrapController::performReset() {
	speedControl1.setSpeed(0);
	motor1->setDirection(-1);
	// check if switch is pressed
	if (switch1->getIfPressed()) {
		speedControl1.setSpeed(0);
		encoder1->resetCount();
		speedControl1.performMovement();
		return true;
	}
	else {
		speedControl1.setSpeed(0.0007);
		speedControl1.performMovement();
		return false;
	}
}

bool ScrapController::set(int g1) {
	goal1 = g1;
	// set a smaller slowdown thresh if this is a small movement
	if (abs(goal1-encoder1->getCount()) <= slowdownGLOBALThresh) {
		slowdownThresh = shortSlowdownThresh;
	}
	else {
		slowdownThresh = slowdownGLOBALThresh;
	}
	powerInit = powerGLOBALInit;
	return checkIfDone();
}

void ScrapController::stop() {
	speedControl1.setSpeed(0);
	speedControl1.performMovement();
}

bool ScrapController::performMovement() {
	//check if already done moving
	if (checkIfDone()) { 
		stop();
		return true;
	}
	//else, gotta do stuff
	else {
		if (encoder1->getCount() < goal1) {
			motor1->setMotor(calcPower1());
		}
		else {
			motor1->setMotor(-calcPower1());
		}
	}
	
	return false;
}

bool ScrapController::performMovementSpeed() {
	if (checkIfDone()) { 
		stop();
		return true;
	}
	//else, gotta do stuff
	else {
		if (encoder1->getCount() < goal1) {
			speedControl1.setControl(calcSpeed1());
		}
		else {
			speedControl1.setControl(-calcSpeed1());
		}
	}
	speedControl1.performMovement();
	return false;
}

// calculate speed to give motor
float ScrapController::calcSpeed1() {
	int diff = getDiff1();
	if (diff > slowdownThresh) {
		return speedControl1.getSpeedGoal();
	}
	else {
		return speedControl1.convertToSpeed(map(diff,1,slowdownThresh,encTolerance,slowdownThresh));
	}
}


// calculate power to give motor
int ScrapController::calcPower1() {
	int diff = getDiff1();
	if (diff > slowdownThresh) {
		return powerInit;
	}
	else {
		return map(diff,0,slowdownThresh,minSlowPower,powerInit);
	}
}

int ScrapController::getDiff1() {
	return abs(encoder1->getCount() - goal1);
}

// increment or decrement target power
void ScrapController::incrementPower(int val) {
	powerInit = min(255,motor1->getPower()+val);
}
// decrement, but with a different slow power than for calcPower
void ScrapController::decrementPower(int val) {
	powerInit = max(minSlowPower,motor1->getPower()-val);
}
// set to powerInit
void ScrapController::resumePower() {
	powerInit = powerGLOBALInit;
}

// check if encoder count is within tolerance of goal
bool ScrapController::checkIfDone1() {
	return (encoder1->getCount() >= goal1 - encTolerance ) && (encoder1->getCount() <= goal1 + encTolerance );
}

// attach motors + encoders to be used
void ScrapController::attachMotor1(ScrapMotor& mot) {
	motor1 = &mot;
}

void ScrapController::attachEncoder1(ScrapEncoder& enc) {
	encoder1 = &enc;
}
