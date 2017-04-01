#include "ScrapController.h"

//ScrapFullController::

ScrapFullController::ScrapFullController() {
	
}

ScrapFullController::ScrapFullController(ScrapController& xCont, ScrapDualController& yCont) {
	attachControllerX(xCont);
	attachControllerY(yCont);
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
	// else, have each controller do its own movement
	bool isDone1 = xControl->performMovement();
	bool isDone2 = yControl->performMovement();
	// balance power to promote proportional movement
	balancePower();
	// report back accordingly
	return isDone1 && isDone2;
	
}


// balance power to maintain proportional movement
void ScrapFullController::balancePower() {
	// if proportion was zero, do not worry about balancing
	if (desiredProportion == 0) {
		return;
	}
	// get current proportion
	float currentProportion = getMovementProportion();
	// if current proportion is greater, then move power to Y
	if (currentProportion > desiredProportion*(1.0+diffDecim)) {
		movePowerTowardX();
	}
	// else, move power to X
	else if (currentProportion < desiredProportion*(1.0-diffDecim)) {
		movePowerTowardY();
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
void ScrapFullController::movePowerTowardX(int val) {
	xControl->incrementPower(val);
	yControl->decrementPower(val);
}

void ScrapFullController::movePowerTowardY(int val) {
	xControl->decrementPower(val);
	yControl->incrementPower(val);
}

// attach controllers
void ScrapFullController::attachControllerX(ScrapController& xCont) {
	xControl = &xCont;
}

void ScrapFullController::attachControllerY(ScrapDualController& yCont) {
	yControl = &yCont;
}
