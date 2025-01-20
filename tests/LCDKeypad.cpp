// LCDKeypad.cpp
// Program to test LCD.
// The LCD display with two lines, 20 characters each.
// PORT A1: PA_15 : PA_8
// There are three control lines (PA_14:PA_12) and four data lines (PA_11 : PA_8).
// PA_14 - RS=0 Data represents Command, RS=1 Data represents Character
// PA_13 - RW=0 Writing into the LCD module
// PA_12 - E =1 Data is latched into LCD module during low to hight transition  
#undef __ARM_FP

#include "mbed.h"
#include "lcd.h"	
#include "keypad.h"	


unsigned char key, outChar;

// ---- Main Program ---------------------------------------------------------------
int main( )
{
	printf("This is MAPP LCDKeypad running on Mbed OS %d.%d.%d.\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

	int i;
	lcd_init();						// Initialise LCD module

    while (1)
    {
        
        lcd_write_string("Please select one of the following");
        lcd_Clear();
        lcd_write_cmd(0x80);            // Move cursor to line 1 position 1
        lcd_write_string("1:Grow Light");
        lcd_write_cmd(0xC0);
        lcd_write_string("2:Feeding frequancy");

        thread_sleep_for(1000);
        lcd_Clear();

        lcd_write_cmd(0x80);            // Move cursor to line 1 position 1
        lcd_write_string("3:Temperature");
        lcd_write_cmd(0xC0);
        lcd_write_string("4:Watering frequancy");

        thread_sleep_for(1000);
        lcd_Clear();

        lcd_write_cmd(0x80);            // Move cursor to line 1 position 1
        lcd_write_string("5:Water level");
        lcd_write_cmd(0xC0);
        lcd_write_string("6:Sensor Values");
        lcd_Clear();
        thread_sleep_for(1000);       	

    }
    key = getkey(); 				// waits and get an ascii key number when pressed
    lcd_write_data(key);		//display on LCD
}




