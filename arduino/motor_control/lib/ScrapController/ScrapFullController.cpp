#include "ScrapController.h"

//ScrapFullController::

ScrapFullController::ScrapFullController() {
	
}

ScrapFullController::ScrapFullController(ScrapController& xCont, ScrapDualController& yCont) {
	attachControllerX(xCont);
	attachControllerY(yCont);
}

// move back until switches are activated
bool ScrapFullController::performReset() {
	// check if each controller is done resetting
	bool xReset = xControl->performReset();
	bool yReset = yControl->performReset();
	if (xReset && yReset) {
		return true;
	}
	else {
		return false;
	}
}

bool ScrapFullController::set(int gx, int gy) {
	xControl->set(gx);
	yControl->set(gy);
	desiredProportion = getMovementProportion();
	return checkIfDone();
}

bool ScrapFullController::performMovement() {
	// if done moving, report back accordingly
	if (checkIfDone()) {
		stop();
		desiredProportion = 0;
		return true;
	}
	// else, balance power to promote proportional movement
	balanceSpeed();
	// have each controller do its own movement
	bool isDone1 = xControl->performMovement();
	bool isDone2 = yControl->performMovement();
	// report back accordingly
	return isDone1 && isDone2;
	
}


// balance speed to maintain proportional movement
void ScrapFullController::balanceSpeed() {
	float prop = 0.1;
	// if proportion was zero, do not worry about balancing
	if (desiredProportion == 0) {
		return;
	}
	// get current proportion
	float currentProportion = getMovementProportion();
	// if current proportion is greater, then move speed to Y
	if (currentProportion*(1.0+diffDecim) > desiredProportion) {
		moveSpeedTowardX(prop);
	}
	// else if opposite, move speed to X
	else if (currentProportion*(1.0-diffDecim) < desiredProportion) {
		moveSpeedTowardY(prop);
	}
}


// calculate current proportion
float ScrapFullController::getMovementProportion() {
	// get x remaining distance
	int x_remains = abs(xControl->getGoal() - xControl->getCount());
	// get y remaining distance
	int y_remains = abs(yControl->getGoal() - yControl->getCount());
	// retrieve proportion, watching out for zero division
	if (y_remains == 0) {
		return 0;
	}
	else {
		return (float)x_remains/(float)y_remains;
	}
}

// move power towards X or Y motors
void ScrapFullController::moveSpeedTowardX(float prop) {
	xControl->incrementSpeed(prop);
	yControl->decrementSpeed(prop);
}

void ScrapFullController::moveSpeedTowardY(float prop) {
	xControl->decrementSpeed(prop);
	yControl->incrementSpeed(prop);
}

// attach controllers
void ScrapFullController::attachControllerX(ScrapController& xCont) {
	xControl = &xCont;
}

void ScrapFullController::attachControllerY(ScrapDualController& yCont) {
	yControl = &yCont;
}
