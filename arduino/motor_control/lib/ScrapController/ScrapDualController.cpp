#include "ScrapController.h"

//ScrapDualController::

ScrapDualController::ScrapDualController() {
	powerMax = powerGLOBALMax;
	powerMin = powerGLOBALMin;
	powerInit1 = powerGLOBALInit;
	powerInit2 = powerGLOBALInit;
}

ScrapDualController::ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2) {
	powerMax = powerGLOBALMax;
	powerMin = powerGLOBALMin;
	powerInit1 = powerGLOBALInit;
	powerInit2 = powerGLOBALInit;
	attachMotor1(mot1);
	attachMotor2(mot2);
	attachEncoder1(enc1);
	attachEncoder2(enc2);
	stop();
}

bool ScrapDualController::set(int g1, int g2) {
	goal1 = g1;
	goal2 = g2;
	// set a smaller slowdown thresh if this is a small movement
	if (abs(goal1-encoder1->getCount()) <= slowdownGLOBALThresh) {
		slowdownThresh = shortSlowdownThresh;
	}
	else {
		slowdownThresh = slowdownGLOBALThresh;
	}
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

// calculate power to give motor
int ScrapDualController::calcPower1() {
	int diff = abs(encoder1->getCount() - goal1);
	if (diff > slowdownThresh) {
		return powerInit1;
	}
	else {
		return map(diff,0,slowdownThresh,minSlowPower1,powerInit1);
	}
}

int ScrapDualController::calcPower2() {
	int diff = abs(encoder2->getCount() - goal2);
	if (diff > slowdownThresh) {
		return powerInit2;
	}
	else {
		return map(diff,0,slowdownThresh,minSlowPower2,powerInit2);
	}
}

void ScrapDualController::stop() {
	motor1->stop();
	motor2->stop();
}

// increment or decrement target power TODO
void ScrapDualController::incrementPower(int val) {
	powerInit1 = min(255,motor1->getPower()+val);
	powerInit2 = min(255,motor2->getPower()+val);
}

void ScrapDualController::decrementPower(int val) {
	powerInit1 = max(minSlowPower1,motor1->getPower()-val);
	powerInit2 = max(minSlowPower2,motor2->getPower()-val);
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

