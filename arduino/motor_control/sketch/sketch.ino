#include <SPI.h>
#include <Wire.h>
#include <Servo.h>
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

#define SWITCH_PIN1 22
#define SWITCH_PIN2 24
#define SWITCH_PIN3 26

#define SERVO_PIN1 30
// pen positions
#define PEN_UP 75
#define PEN_DOWN 10

// LCD DEFINITIONS
Adafruit_SSD1306 display(-1);

ScrapEncoder encoderLeftY(ENC1PINA,ENC1PINB);
ScrapEncoder encoderRightY(ENC2PINA,ENC2PINB);
ScrapEncoder encoderAxisX(ENC3PINA,ENC3PINB);
ScrapMotor motorLeftY(MOT1_PIND1,MOT1_PIND2,MOT1_PINPWM,-1); //flip direction
ScrapMotor motorRightY(MOT2_PIND1,MOT2_PIND2,MOT2_PINPWM);
ScrapMotor motorAxisX(MOT3_PIND1,MOT3_PIND2,MOT3_PINPWM);
ScrapSwitch switchLeftY(SWITCH_PIN1);
ScrapSwitch switchRightY(SWITCH_PIN2);
ScrapSwitch switchAxisX(SWITCH_PIN3);
// create ScrapDualController
ScrapDualController dualControl(motorLeftY,motorRightY,encoderLeftY,encoderRightY,switchLeftY,switchRightY);
// create ScrapController
ScrapController uniControl(motorAxisX,encoderAxisX,switchAxisX);
// finall create ScrapFullController
ScrapFullController fullControl(uniControl,dualControl);
// create servo
Servo servoPen;


// delay time
const int delayTime = 2;
// used for setting goal;
int setGoal;

// input parsing
const int maxValues = 2;
String command; // general command
String values[maxValues]; // stores values for command
String response; // response returned to main program


void setup() {
	//initialize encoders
	initEncoders();
	//initialize servo
	servoPen.attach(SERVO_PIN1);
	servoPen.write(PEN_UP);
	//display stuff
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.display();
	display.clearDisplay();
	showText("PrinterScraps");
	// reset arm
	showText("resetting arm");
	performReset();
	// set initial goal
	setGoal = 0;
	showText("setting goals");
	fullControl.set(setGoal,setGoal);
	showText("ready!");
	// start serial and signal to main program that we are connected
	Serial.begin(115200);
	Serial.write(1);
}


void loop () {
	// perform movement - moves arm to some goal coordinate
	performActions();

	// if something in serial, parse it
	if(Serial.available()){

		int addTo = 0; // 0 for command, 1 for value

		while (Serial.available() > 0)
		{
			char readIn = (char)Serial.read();
			if (readIn == '\n') {
				break;
			}
			else if (readIn == '|') {
				addTo += 1;
				continue;
			}
			// add to command if no | reached yet
			if (addTo == 0) {
				command += readIn;
			}
			// add to proper value in array
			else if (addTo <= maxValues) {
				values[addTo-1] += readIn;
			}
			// if values exceed max, then stop listening to prevent problems
			else {
				break;
			}
		}
		//clear anything remaining in serial
		while (Serial.available() > 0) {
			Serial.read();
		}
		response = interpretCommand();
		Serial.println(response); //sends response with \n at the end
		// empty out command and value strings
		command = "";
		for (int i = 0; i < maxValues; i++) {
			values[i] = "";
		}
	}
	// show statuses
	showText(String(uniControl.getGoal())+','+String(dualControl.getGoal1()) 
	+'\n' + String(dualControl.getCount1())+'\n'+String(dualControl.getCount2())
	+'\n'+String(uniControl.getCount1()));
	// small delay
	delay(delayTime);
}

String interpretCommand() {
	String responseString = "n";  // string to be sent to main program
	String returnString = "";     // string received from a subfunction

	// determine what to do:
	
	// check if movement command - make sure length is okay
	if (command == "s") {
		if (values[0].length() != 4 || values[1].length() != 4)
			return responseString;
		responseString = "1";
		returnString = performSet(values[0].toInt(),values[1].toInt());
	}
	else if (command == "sp") {
		if (values[0].length() != 4 || values[1].length() != 4)
			return responseString;
		responseString = "1";
		returnString = performSetPassive(values[0].toInt(),values[1].toInt());
	}
	else if (command == "r") {
		responseString = "1";
		returnString = performReset();
	}
	else if (command == "u") {
		responseString = "1";
		returnString = performPenUp();
	}
	else if (command == "d") {
		responseString = "1";
		returnString = performPenDown();
	}
	// check if mode-setting command
	// TODO: add functionality to STREAMOUT
	else if (command == "SYNCOUT") {
		responseString = "1";
	}
	else if (command == "STREAMOUT") {
		responseString = "1";
	}

	responseString += returnString;
	return responseString;
}

// set x and y coordinates
String performSet(const int& x_coord, const int& y_coord) {
	fullControl.set(x_coord,y_coord);
	while (!performActions()) {
		delay(delayTime);
	}
	return "1";
}

// set x and y, but do not wait for completion
String performSetPassive(const int& x_coord, const int& y_coord) {
	fullControl.set(x_coord,y_coord);
	return "1";
}


String performReset() {
	// move goal to 0,0 to forget previous goal
	fullControl.set(0,0);
	while (!fullControl.performReset()) {
		delay(delayTime);
	}
	return "1";
}

String performPenDown() {
	servoPen.write(PEN_DOWN);
	return "1";
}

String performPenUp() {
	servoPen.write(PEN_UP);
	return "1";
}

// perform all necessary movement to reach goal
bool performActions() {
	return fullControl.performMovement();
}

// used for testing in general
void performRandom() {
	showText(String(setGoal) + '\n' + String(dualControl.getCount1())+'\n'+String(dualControl.getCount2())+'\n'+String(uniControl.getCount1()));
	bool isDone = fullControl.performMovement();
	if (isDone) {
		setGoal = getRandom();
		fullControl.set(setGoal,setGoal);
	}
	delay(delayTime);
}

// generate random number to set as goal
int getRandom() {
	return random(200,3500);
}

// initialize encoders
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
		encoderAxisX.decrementCount();
	}
	else {
		encoderAxisX.incrementCount();
	}
}

// display text on OLED display
void showText(String text) {
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.println(text);
	display.display();
	display.clearDisplay();
}
