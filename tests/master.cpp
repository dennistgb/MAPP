#include <cstdio>
#undef __ARM_FP

#include "mbed.h"
#include "lcd.h"
#include "keypad.h"

// -------------------------
// UART to Slave
// -------------------------
// Master TX = PB_6, Master RX = PB_7, 9600 baud
UnbufferedSerial slave_uart(PB_6, PB_7, 9600);

// Simple helper to send commands to the Slave
void send_command(const char *cmd_str) {
    // We assume cmd_str ends with no newline; we add one
    // for the Slave to detect command boundaries easily.
    char buffer[16];
    int len = sprintf(buffer, "%s", cmd_str);
    slave_uart.write(buffer, len);
}

// -------------------------
// Keypad & LCD
// -------------------------
extern BusIn Keypad_Data;
InterruptIn keypad_interrupt(PB_13);         // DA pin of the keypad for interrupt

const unsigned char lookupTable[] = {
    '1', '2', '3', 'F',
    '4', '5', '6', 'E',
    '7', '8', '9', 'D',
    'A', '0', 'B', 'C'
};

unsigned char key;

void keypad_ISR();
void display_menu(const char* line1, const char* line2);

// Display two lines on the LCD
void display_menu(const char* line1, const char* line2) {
    printf("im here!");
    lcd_write_cmd(0x01);
    lcd_write_cmd(0x01);
    lcd_write_cmd(0x01);
    lcd_write_cmd(0x01);
    lcd_write_cmd(0x01);  
    lcd_write_cmd(0x80);  // Move cursor to line 1 position 1
    lcd_write_string(line1);
    lcd_write_cmd(0xC0);  // Move cursor to line 2 position 1
    lcd_write_string(line2);
}

int main() {
    printf("Master Code with Keypad & LCD.\n");

    // Initialize LCD
    lcd_init();       
    lcd_Clear();      

    // Configure keypad data pins
    Keypad_Data.mode(PullNone);

    // Attach interrupt on falling edge of DA pin
    keypad_interrupt.fall(&keypad_ISR);

    // Show a rotating menu for demonstration
    while (1) {
        display_menu("1:Bright Low",   "2:Bright Default");
        thread_sleep_for(1000);

        display_menu("3:Bright High",  "4:Moist Low");
        thread_sleep_for(1000);

        display_menu("5:Moist Default","6:Moist High");
        thread_sleep_for(1000);
    }
}

// Keypad Interrupt Service Routine
void keypad_ISR() {
    // Read the 4-bit value from the bus (PB_8..PB_11)
    unsigned char keycode = Keypad_Data & Keypad_Data.mask();

    // Convert the keycode to its corresponding character
    key = lookupTable[keycode];
    printf("Key pressed: '%c'\n", key);
    printf("boomz");
    lcd_write_cmd(0x01);

    // Clear and display the key info on LCD
    printf("oopsies");
    switch (key) 
    {
        case '1':
            // "BL" = Brightness Low
            send_command("BL");
            lcd_write_string("Brightness Low");
            printf("Sent command: BL\n");
            break;
        case '2':
            // "BD" = Brightness Default
            send_command("BD");
            lcd_write_string("Brightness Default");
            printf("Sent command: BD\n");
            break;
        case '3':
            // "BH" = Brightness High
            send_command("BH");
            lcd_write_string("Brightness High");
            printf("Sent command: BH\n");
            break;
        case '4':
            // "ML" = Moisture Low
            send_command("ML");
            lcd_write_string("Moisture Low");
            printf("Sent command: ML\n");
            break;
        case '5':
            // "MD" = Moisture Default
            send_command("MD");
            lcd_write_string("Moisture Default");
            printf("Sent command: MD\n");
            break;
        case '6':
            // "MH" = Moisture High
            send_command("MH");
            lcd_write_string("Moisture High");
            printf("Sent command: MH\n");
            break;
        default:
            lcd_write_string("Invalid Input");
            printf("Invalid Input\n");
            break;
    }
    printf("im out!!!");
    // Pause so user can see the LCD message
}
