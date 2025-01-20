#include <cstdio>
#undef __ARM_FP

#include "mbed.h"
#include "lcd.h"
#include "keypad.h"  

unsigned char key;

void display_menu(const char* line1, const char* line2) {
    lcd_Clear();
    lcd_write_cmd(0x80);  // Move cursor to line 1 position 1
    lcd_write_string(line1);
    lcd_write_cmd(0xC0);  // Move cursor to line 2 position 1
    lcd_write_string(line2);
}

int main() {
   printf("the code is starting \n");

    lcd_init(); // Initialize LCD module
    lcd_Clear();

    while (1) {
        // Cycle through menus
        display_menu("1:Grow Light", "2:Feeding Freq");
        thread_sleep_for(1000);
        lcd_Clear();

        display_menu("3:Temperature", "4:Watering Freq");
        thread_sleep_for(1000);
        lcd_Clear();

        display_menu("5:Water Level", "6:Sensor Values");
        thread_sleep_for(1000);
        lcd_Clear();

        // Wait for key press and handle action
        key = getkey_nowait();
    //    key = getkey();

        // Handle the key press scenario
        switch (key) {
            case '1':
                lcd_Clear();
                lcd_write_string("Grow Light");
                printf("Action: Grow Light\n");
                break;
            case '2':
                lcd_Clear();
                lcd_write_string("Feeding Freq");
                printf("Action: Feeding Frequency\n");
                break;
            case '3':
                lcd_Clear();
                lcd_write_string("Temperature");
                printf("Action: Temperature\n");
                break;
            case '4':
                lcd_Clear();
                lcd_write_string("Watering Freq");
                printf("Action: Watering Frequency\n");
                break;
            case '5':
                lcd_Clear();
                lcd_write_string("Water Level");
                printf("Action: Water Level\n");
                break;
            case '6':
                lcd_Clear();
                lcd_write_string("Sensor Values");
                printf("Action: Sensor Values\n");
                break;
            default:
                lcd_Clear();
                lcd_write_string("Invalid Input");
                printf("Action: Invalid Input\n");
                break;
        }

        // Pause for 2 seconds before returning to the menu
        thread_sleep_for(2000);
    }
}
