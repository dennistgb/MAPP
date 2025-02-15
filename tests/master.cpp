#include <cstdio>
#undef __ARM_FP

#include "mbed.h"
#include "lcd.h"
#include "keypad.h"

// UART communication
UnbufferedSerial slave_uart(PB_6, PB_7, 9600);

// Keypad & LCD
extern BusIn Keypad_Data;
InterruptIn keypad_interrupt(PB_13); // Keypad DA

// Shared state for ISR communication
volatile bool key_pressed = false;
volatile unsigned char key;

// UART helper
void send_command(const char *cmd_str) {
    char buffer[4];
    int len = sprintf(buffer, "%s\n", cmd_str);
    slave_uart.write(buffer, len);
}

// Key mapping
const unsigned char lookupTable[] = {
    '1', '2', '3', 'F',
    '4', '5', '6', 'E',
    '7', '8', '9', 'D',
    'A', '0', 'B', 'C'
};

// ISR
void keypad_ISR() {
    unsigned char keycode = Keypad_Data & Keypad_Data.mask();
    key = lookupTable[keycode];
    key_pressed = true;
}

// LCD Helpers
void display_menu(const char* line1, const char* line2) {
    lcd_write_cmd(0x01);
    lcd_write_cmd(0x80);
    lcd_write_string(line1);
    lcd_write_cmd(0xC0);
    lcd_write_string(line2);
}

void display_key_action(char key) {
    lcd_write_cmd(0x01);
    switch (key) {
        case '1':
            send_command("TL");
            lcd_write_string("Temperature Low");
            break;
        case '2':
            send_command("TD");
            lcd_write_string("Temperature Default");
            break;
        case '3':
            send_command("TH");
            lcd_write_string("Temperature High");
            break;
        case '4':
            send_command("ML");
            lcd_write_string("Moisture Low");
            break;
        case '5':
            send_command("MD");
            lcd_write_string("Moisture Default");
            break;
        case '6':
            send_command("MH");
            lcd_write_string("Moisture High");
            break;
        default:
            lcd_write_string("Invalid Input");
            break;
    }
}

int main() {
    lcd_init();
    lcd_Clear();

    Keypad_Data.mode(PullNone);
    keypad_interrupt.fall(&keypad_ISR);

    int menu_index = 0;
    const char* menus[][2] = {
        {"1:Temp Low", "2:Temp Default"},
        {"3:Temp High", "4:Moist Low"},
        {"5:Moist Default", "6:Moist High"},
    };
    const int menu_count = 3;

    while (true) {
        if (key_pressed) {
            unsigned char pressed_key = key;  // Read key from ISR
            key_pressed = false;              // Clear the flag

            display_key_action(pressed_key);  // Show selected action on LCD
            thread_sleep_for(2000);           // Let the user see the result
        } else {
            display_menu(menus[menu_index][0], menus[menu_index][1]);

            // Small intervals, so the menu can be interrupted quickly
            int wait_time = 1000;
            int step_time = 50;
            for (int i = 0; i < wait_time / step_time; i++) {
                if (key_pressed) break;
                thread_sleep_for(step_time);
            }

            if (!key_pressed) {
                menu_index = (menu_index + 1) % menu_count;
            }
        }
    }
}