#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#include "ScrapController.h"


#define ENC1PINA 17
#define ENC1PINB 15
#define ENC2PINA 18
#define ENC2PINB 16

// LCD DEFINITIONS
Adafruit_SSD1306 display();

ScrapEncoder encoderLeft;
ScrapEncoder encoderRight;



void setup() {
	encoderLeft = ScrapEncoder(ENC1PINA,ENC1PINB);
	encoderRight = ScrapEncoder(ENC2PINA,ENC2PINA);
	//display stuff
	display.begin(SSD1306_SWITCHCAPVCC, Ox3C);
	display.display();
	display.clearDisplay();
	showText("PrinterScraps");
}

void loop () {
	delay(500);
	// show encoder counts
	showText(String(encoderLeft.getCount())+'\n'+String(encoderRight.getCount()));
}

void showText(String text) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(text);
  display.display();
  display.clearDisplay();
}