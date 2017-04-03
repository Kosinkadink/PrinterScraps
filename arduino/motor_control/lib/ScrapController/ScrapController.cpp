#include "ScrapController.h"

//ScrapController::

ScrapController::ScrapController() {
	powerInit = powerGLOBALInit;
}

ScrapController::ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1) {
	powerInit = powerGLOBALInit;
	attachMotor1(mot1);
	attachEncoder1(enc1);
	stop();
}

ScrapController::ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1, ScrapSwitch& swi1) {
	ScrapController(mot1,enc1);
	attachSwitch1(swi1);
}

// move back until switches are activated
bool ScrapController::performReset() {
	// check if switch is pressed
	if (switch1->getIfPressed()) {
		motor1->stop();
		encoder1->resetCount();
		return true;
	}
	else {
		motor1->setMotor(-150);
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

// calculate power to give motor
int ScrapController::calcPower1() {
	int diff = abs(encoder1->getCount() - goal1);
	if (diff > slowdownThresh) {
		return powerInit;
	}
	else {
		return map(diff,0,slowdownThresh,minSlowPower,powerInit);
	}
}

// increment or decrement target power
void ScrapController::incrementPower(int val) {
	powerInit = min(255,motor1->getPower()+val);
}
// decrement, but with a different slow power than for calcPower
void ScrapController::decrementPower(int val) {
	powerInit = max(minSlowPower,motor1->getPower()-val);
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
