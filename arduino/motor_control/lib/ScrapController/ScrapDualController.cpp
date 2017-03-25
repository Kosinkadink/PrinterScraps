#include "ScrapController.h"

//ScrapDualController::

ScrapDualController::ScrapDualController() {

}

ScrapDualController::ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2) {
	attachMotor1(mot1);
	attachMotor2(mot2);
	attachEncoder1(enc1);
	attachEncoder2(enc2);
}

bool set(int g1, int g2) {
	goal1 = g1;
	goal2 = g2;
	return checkIfDone();
}

bool performMovement() {
	//check if already done moving
	if (checkIfDone())
		return true;
	//else, gotta do stuff

}

bool checkIfDone() {
	return (checkIfDone1() && checkIfDone2());
}

bool checkIfDone1() {
	return (encoder1->getCount() - encTolerance >= goal1) && (encoder1->getCount() + encTolerance <= goal1);
}

bool checkIfDone2() {
	return (encoder2->getCount() - encTolerance >= goal2) && (encoder2->getCount() + encTolerance <= goal2);
}

// attach motors + encoders to be used
ScrapDualController::attachMotor1(ScrapMotor& mot) {
	motor1 = &mot;
}

ScrapDualController::attachMotor2(ScrapMotor& mot) {
	motor2 = &mot;
}

ScrapDualController::attachEncoder1(ScrapEncoder& enc) {
	encoder1 = &enc;
}


ScrapDualController::attachEncoder2(ScrapEncoder& enc) {
	encoder2 = &enc;
}

