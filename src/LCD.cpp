
#include "LCD.h" 

#define TEMPERATURE 0
#define DEGREE 1
#define CLOCK 2
#define WIFI 3

#define PLAY 4
#define PAUSE 5

uint8_t temperature[8] = {
	0b00100,
	0b01010,
	0b01010,
	0b01010,
	0b01010,
	0b10001,
	0b10001,
	0b01110
};

uint8_t degrees[8] = {
	0b01100,
	0b10010,
	0b10010,
	0b01100,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

uint8_t clock[8] = {
	0b00000,
	0b01110,
	0b10101,
	0b10111,
	0b10001,
	0b01110,
	0b00000,
	0b00000
};

uint8_t wifi[8] = {
	0b00000,
	0b01010,
	0b10001,
	0b10101,
	0b10001,
	0b01010,
	0b00000,
	0b00000
};

uint8_t play[8] = {
	0b00000,
	0b01000,
	0b01100,
	0b01110,
	0b01100,
	0b01000,
	0b00000,
	0b00000
};

uint8_t pause[8] = {
	0b00000,
	0b11011,
	0b11011,
	0b11011,
	0b11011,
	0b11011,
	0b11011,
	0b00000
};

const char* stateStr[] = {"Off", "On", "Target PID", "Calibrate", "Reflow", "Calibrate Cool", "Reflow Cool"};

Display::Display() :
groovelcd() {
}

bool Display::setup_LCD() {
	groovelcd.begin(16, 2);
	groovelcd.noDisplay();
	groovelcd.clear();
	groovelcd.display();
	groovelcd.noBlinkLED();
	groovelcd.setColor(GREEN);

	groovelcd.createChar(TEMPERATURE, temperature);
	groovelcd.createChar(DEGREE, degrees);
	groovelcd.createChar(CLOCK, clock);
	groovelcd.createChar(PLAY, play);
	groovelcd.createChar(PAUSE, pause);

	return true;
}

void Display::printf(const char * format, ...) {
	groovelcd.clear();
	va_list args;
	va_start (args, format);	
	groovelcd.printf(format, args);	
	va_end (args);
}

void Display::print(const String& value) {
	groovelcd.clear();
	groovelcd.print(value);	
}

void Display::splash() {
	groovelcd.clear();
	groovelcd.setCursor(3,0);
	groovelcd.setColor(GREEN);
	groovelcd.print("ESP-Forge");
	groovelcd.setCursor(3,1);	
	groovelcd.print("by amd989");	
	delay(5000);
	groovelcd.clear();
}


void Display::displayTemperature(float temperature, float target) {	
	groovelcd.setCursor(0,0);
	groovelcd.write((uint8_t)TEMPERATURE);
	groovelcd.print(temperature);
	groovelcd.print("/");
	groovelcd.print((uint16_t)target);
	groovelcd.write((uint8_t)DEGREE);	
	groovelcd.print("    ");
}

void Display::displaySteps(bool running, const String& name) {	
	groovelcd.setCursor(0,1);
	
	if(running)
		groovelcd.write((uint8_t)PLAY);
	else
		groovelcd.write((uint8_t)PAUSE);

	groovelcd.print(" ");
	groovelcd.print(name);	
	groovelcd.print("    ");
}
