#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#include "ScrapController.h"

#define ENC1PINA 18
#define ENC1PINB 16
#define ENC2PINA 19
#define ENC2PINB 17

#define MOT1_PIND1 40
#define MOT1_PIND2 42
#define MOT1_PINPWM 44

#define MOT2_PIND1 41
#define MOT2_PIND2 43
#define MOT2_PINPWM 45


// LCD DEFINITIONS
Adafruit_SSD1306 display(-1);

ScrapEncoder encoderLeft(ENC1PINA,ENC1PINB);
ScrapEncoder encoderRight(ENC2PINA,ENC2PINA);
ScrapMotor motorLeftY(MOT1_PIND1,MOT1_PIND2,MOT1_PINPWM);
ScrapMotor motorRightY(MOT2_PIND1,MOT2_PIND2,MOT2_PINPWM);
// create ScrapDualController
ScrapDualController dualControl(motorLeftY,motorRightY,encoderLeft,encoderRight);


void setup() {
	//initialize encoders
	initEncoders();
	//display stuff
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.display();
	display.clearDisplay();
	showText("PrinterScraps");
}

void initEncoders() {
	attachInterrupt(digitalPinToInterrupt(ENC1PINA),encoderLeftFunc,CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENC2PINA),encoderRightFunc,CHANGE);
}

void encoderLeftFunc() {
	//encoderLeft.checkEncoder();
	if (digitalRead(ENC1PINA) == digitalRead(ENC1PINB)) {
		encoderLeft.decrementCount();
	}
	else {
		encoderLeft.incrementCount();
	}
}


void encoderRightFunc() {
	//encoderRight.checkEncoder();
	if (digitalRead(ENC2PINA) == digitalRead(ENC2PINB)) {
		encoderRight.incrementCount();
	}
	else {
		encoderRight.decrementCount();
	}
}


void loop () {
	delay(50);
	// show encoder counts
	//showText(String(encoderLeft.getCount())+'\n'+String(encoderRight.getCount()));
	showText(String(dualControl.getCount1())+'\n'+String(dualControl.getCount2())+'\n'+String(dualControl.getCount()));
}


void showText(String text) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(text);
  display.display();
  display.clearDisplay();
}
