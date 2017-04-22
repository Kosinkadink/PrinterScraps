#include "ScrapController.h"

//ScrapDualController::

ScrapDualController::ScrapDualController() {

}

ScrapDualController::ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2) {
	attachMotor1(mot1);
	attachMotor2(mot2);
	attachEncoder1(enc1);
	attachEncoder2(enc2);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	speedControl2 = ScrapMotorControl(*motor2,*encoder2);
	speedControl1.setMinPower(minSlowPower1);
	speedControl1.setMinSpeed(speedControl1.convertToSpeed(minEncSpeed));
	speedControl1.setMaxSpeed(speedControl1.convertToSpeed(maxEncSpeed));
	speedControl2.setMinPower(minSlowPower2);
	speedControl2.setMinSpeed(speedControl2.convertToSpeed(minEncSpeed));
	speedControl2.setMaxSpeed(speedControl2.convertToSpeed(maxEncSpeed));
	stop();
}

ScrapDualController::ScrapDualController(ScrapMotor& mot1, ScrapMotor& mot2, ScrapEncoder& enc1, ScrapEncoder& enc2, ScrapSwitch& swi1, ScrapSwitch& swi2) {
	attachMotor1(mot1);
	attachMotor2(mot2);
	attachEncoder1(enc1);
	attachEncoder2(enc2);
	speedControl1 = ScrapMotorControl(*motor1,*encoder1);
	speedControl2 = ScrapMotorControl(*motor2,*encoder2);
	speedControl1.setMinPower(minSlowPower1);
	speedControl1.setMinSpeed(speedControl1.convertToSpeed(minEncSpeed));
	speedControl1.setMaxSpeed(speedControl1.convertToSpeed(maxEncSpeed));
	speedControl2.setMinPower(minSlowPower2);
	speedControl2.setMinSpeed(speedControl2.convertToSpeed(minEncSpeed));
	speedControl2.setMaxSpeed(speedControl2.convertToSpeed(maxEncSpeed));
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
	// based on separation, balance speed of each motor
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
void ScrapDualController::incrementSpeed(int speedEncDiff) {
	speedControl1.incrementSpeed(speedEncDiff);
	speedControl2.incrementSpeed(speedEncDiff);
}

void ScrapDualController::decrementSpeed(int speedEncDiff) {
	speedControl1.decrementSpeed(speedEncDiff);
	speedControl2.decrementSpeed(speedEncDiff);
}

void ScrapDualController::balanceSpeed() {
	float common_speed;
	// if 1 too far ahead, balance power towards 2
	if ((encoder1->getCount() - encoder2->getCount())*motor1->getDirection() >= diffTolerance) {
		moveSpeedToward2(encSpeedBalance);
	}
	// if 2 too far ahead, balance power towards 1
	else if ((encoder1->getCount() - encoder2->getCount())*motor1->getDirection() <= -diffTolerance) {
		moveSpeedToward1(encSpeedBalance);
	}
	// otherwise, make speeds match if possible
	else {
		if (speedControl1.getSpeedGoal() != 0 && speedControl1.getSpeedGoal() != 0) {
			common_speed = (speedControl1.getSpeedGoal() + speedControl2.getSpeedGoal())/2.0;
			speedControl1.setSpeed(common_speed);
			speedControl2.setSpeed(common_speed);
		}
	}
}

// balance speed
void ScrapDualController::moveSpeedToward1(int speedEncDiff) {
	speedControl1.incrementSpeed(speedEncDiff);
	speedControl2.decrementSpeed(speedEncDiff);
}

void ScrapDualController::moveSpeedToward2(int speedEncDiff) {
	speedControl1.decrementSpeed(speedEncDiff);
	speedControl2.incrementSpeed(speedEncDiff);
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

