#include "ScrapController.h"

//ScrapDualController::

ScrapDualController::ScrapDualController() {
	powerMax = powerGLOBALMax;
	powerMin = powerGLOBALMin;
	powerInit = powerGLOBALInit;
}

ScrapDualController::ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2) {
	powerMax = powerGLOBALMax;
	powerMin = powerGLOBALMin;
	powerInit = powerGLOBALInit;
	attachMotor1(mot1);
	attachMotor2(mot2);
	attachEncoder1(enc1);
	attachEncoder2(enc2);
	stop();
}

bool ScrapDualController::set(int g1, int g2) {
	goal1 = g1;
	goal2 = g2;
	return checkIfDone();
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
	
	return false;

}

// calculate power to give motor
int ScrapDualController::calcPower1() {
	int diff = abs(encoder1->getCount() - goal1);
	if (diff > slowdownThresh) {
		return powerInit;
	}
	else {
		return map(diff,0,slowdownThresh,minSlowPower1,powerInit);
	}
}

int ScrapDualController::calcPower2() {
	int diff = abs(encoder2->getCount() - goal2);
	if (diff > slowdownThresh) {
		return powerInit;
	}
	else {
		return map(diff,0,slowdownThresh,minSlowPower2,powerInit);
	}
}

void ScrapDualController::stop() {
	motor1->stop();
	motor2->stop();
}

// increment or decrement target power
bool ScrapDualController::incrementPower(int val) {
	return false;
}

bool ScrapDualController::decrementPower(int val) {
	return false;
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

