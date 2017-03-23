#include "ScrapController.h"


ScrapEncoder::ScrapEncoder(int pinA, int pinB) {
	PINA_INTERRUPT = pinA;
	PINB = pinB;
	initEncoder();
}


void ScrapEncoder::initEncoder() {
	// initializes encoder pins and count
	pinMode(PINA_INTERRUPT,INPUT);
	pinMode(PINB,INPUT);
	resetCount();
	attachInterrupt(digitalPinToInterrupt(PINA_INTERRUPT),checkEncoder,CHANGE);
}


int ScrapEncoder::getCount() {
	return encCount;
}


void ScrapEncoder::resetCount() {
	// reset encCount to 0
	encCount = 0;
}


void ScrapEncoder::checkEncoder() {
	// check encoder and increment value accordingly
	// this is the function to attach to an interrupt
	if (digitalRead(PINA_INTERRUPT) == digitalRead(PINB)) {
		encCount++;
	}
	else {
		encCount--;
	}
}