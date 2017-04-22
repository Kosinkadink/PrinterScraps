#include "ScrapController.h"

//ScrapController::

ScrapController::ScrapController() {
	
}

ScrapController::ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1) {
	attachMotor1(mot1);
	attachEncoder1(enc1);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	speedControl1.setMinPower(minSlowPower);
	speedControl1.setMinSpeed(speedControl1.convertToSpeed(minEncSpeed));
	speedControl1.setMaxSpeed(speedControl1.convertToSpeed(maxEncSpeed));
	stop();
}

ScrapController::ScrapController(ScrapMotor& mot1, ScrapEncoder& enc1, ScrapSwitch& swi1) {
	attachMotor1(mot1);
	attachEncoder1(enc1);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	speedControl1.setMinPower(minSlowPower);
	speedControl1.setMinSpeed(speedControl1.convertToSpeed(minEncSpeed));
	speedControl1.setMaxSpeed(speedControl1.convertToSpeed(maxEncSpeed));
	attachSwitch1(swi1);
	stop();
}

// move back until switches are activated
bool ScrapController::performReset() {
	speedControl1.stop();
	motor1->setDirection(-1);
	// check if switch is pressed
	if (switch1->getIfPressed()) {
		speedControl1.stop();
		encoder1->resetCount();
		speedControl1.performMovement();
		return true;
	}
	else {
		speedControl1.setSpeed(0.0011);
		speedControl1.performMovement();
		return false;
	}
}

bool ScrapController::set(int g1) {
	goal1 = g1;
	return checkIfDone();
}

void ScrapController::stop() {
	speedControl1.stop();
	speedControl1.performMovement();
}

bool ScrapController::performMovement() {
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
	return speedControl1.convertToSpeed(map(diff,1,slowdownThresh,minEncSpeed,maxEncSpeed));
}

int ScrapController::getDiff1() {
	return abs(encoder1->getCount() - goal1);
}

// increment or decrement speed
void ScrapController::incrementSpeed(int speedEncDiff) {
	speedControl1.incrementSpeed(speedEncDiff);
}

void ScrapController::decrementSpeed(int speedEncDiff) {
	speedControl1.decrementSpeed(speedEncDiff);
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
