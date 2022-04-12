/**
 *  Copyright (C) 2018  foxis (Andrius Mikonis <andrius.mikonis@gmail.com>)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include "rgb_lcd.h"

class Display {

public:
	rgb_lcd groovelcd;
    	
public:
	Display();
	bool setup_LCD();
	void printf(const char * format, ...);
	void print(const String& value);
	
	void splash();

	void displayTemperature(float temperature, float target);
	void displaySteps(bool running, const String& name);
};

#endif
