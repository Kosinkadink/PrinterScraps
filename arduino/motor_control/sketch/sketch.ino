#define LEDpin 13

bool isOn;

void setup() {
	pinMode(LEDpin,OUTPUT);
	digitalWrite(LEDpin, LOW);
	isOn = false;
}

void loop() {
	if (isOn) {
		digitalWrite(LEDpin, LOW);
	}
	else {
		digitalWrite(LEDpin, HIGH);
	}
	delay(500);	
}