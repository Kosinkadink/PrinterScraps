#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#include "ScrapController.h"

#define ENC1PINA 18
#define ENC1PINB 16
#define ENC2PINA 19
#define ENC2PINB 17
#define ENC3PINA 2
#define ENC3PINB 4

#define MOT1_PIND1 40
#define MOT1_PIND2 42
#define MOT1_PINPWM 44

#define MOT2_PIND1 41
#define MOT2_PIND2 43
#define MOT2_PINPWM 45

#define MOT3_PIND1 50
#define MOT3_PIND2 48
#define MOT3_PINPWM 46


// LCD DEFINITIONS
Adafruit_SSD1306 display(-1);

ScrapEncoder encoderLeftY(ENC1PINA,ENC1PINB);
ScrapEncoder encoderRightY(ENC2PINA,ENC2PINB);
ScrapEncoder encoderAxisX(ENC3PINA,ENC3PINB);
ScrapMotor motorLeftY(MOT1_PIND1,MOT1_PIND2,MOT1_PINPWM,-1); //flip direction
ScrapMotor motorRightY(MOT2_PIND1,MOT2_PIND2,MOT2_PINPWM);
ScrapMotor motorAxisX(MOT3_PIND1,MOT3_PIND2,MOT3_PINPWM);
// create ScrapDualController
ScrapDualController dualControl(motorLeftY,motorRightY,encoderLeftY,encoderRightY);
// create ScrapController
ScrapController uniControl(motorAxisX,encoderAxisX);

int pwmChosen = 150;
int setGoal;

void setup() {
	//initialize encoders
	initEncoders();
	//display stuff
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.display();
	display.clearDisplay();
	showText("PrinterScraps");
	setGoal = getRandom();
	dualControl.set(setGoal,setGoal);
	uniControl.set(setGoal);
}


void loop () {
	showText(String(setGoal) + '\n' + String(dualControl.getCount1())+'\n'+String(dualControl.getCount2())+'\n'+String(uniControl.getCount1()));
	bool isDone = dualControl.performMovement();
	bool isDone2 = uniControl.performMovement();
	if (isDone) {
		setGoal = getRandom();
		dualControl.set(setGoal,setGoal);
	}
	delay(5);
	
}


int getRandom() {
	return random(200,3500);
}


void initEncoders() {
	attachInterrupt(digitalPinToInterrupt(ENC1PINA),encoderLeftYFunc,CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENC2PINA),encoderRightYFunc,CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENC3PINA),encoderAxisXFunc,CHANGE);
}


void encoderLeftYFunc() {
	//encoderLeftY.checkEncoderFlipped();
	if (digitalRead(ENC1PINA) == digitalRead(ENC1PINB)) {
		encoderLeftY.decrementCount();
	}
	else {
		encoderLeftY.incrementCount();
	}
}


void encoderRightYFunc() {
	//encoderRightY.checkEncoder();
	if (digitalRead(ENC2PINA) == digitalRead(ENC2PINB)) {
		encoderRightY.incrementCount();
	}
	else {
		encoderRightY.decrementCount();
	}
}


void encoderAxisXFunc() {
	//encoderAxisX.checkEncoder();
	if (digitalRead(ENC3PINA) == digitalRead(ENC3PINB)) {
		encoderAxisX.incrementCount();
	}
	else {
		encoderAxisX.decrementCount();
	}
}


void showText(String text) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(text);
  display.display();
  display.clearDisplay();
}
