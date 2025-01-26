// #include <cstdio>
// #undef __ARM_FP

// #include "mbed.h"
// #include "lcd.h"
// #include "keypad.h"

// unsigned char key;
// BusIn Keypad_Data(PB_8, PB_9, PB_10, PB_11); // Keypad data pins
// InterruptIn keypad_interrupt(PB_13);         // DA pin of the keypad for interrupt

// const unsigned char lookupTable[] = {'1', '2', '3', 'F', '4', '5', '6', 'E', '7', '8', '9', 'D', 'A', '0', 'B', 'C'};

// // Function prototypes
// void keypad_ISR();
// void display_menu(const char* line1, const char* line2);

// void display_menu(const char* line1, const char* line2) {
//     lcd_Clear();
//     lcd_write_cmd(0x80);  // Move cursor to line 1 position 1
//     lcd_write_string(line1);
//     lcd_write_cmd(0xC0);  // Move cursor to line 2 position 1
//     lcd_write_string(line2);
// }

// int main() {
//     printf("This is MAPP LCDKeypad running on Mbed OS %d.%d.%d.\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

//     lcd_init();       // Initialize LCD module
//     lcd_Clear();      // Clear the LCD screen

//     // Configure keypad data pins
//     Keypad_Data.mode(PullNone);

//     // Attach interrupt to keypad DA pin
//     keypad_interrupt.fall(&keypad_ISR);

//     while (1) {
//         // Continuously cycle through the menu options
//         display_menu("1:Grow Light", "2:Feeding Freq");
//         thread_sleep_for(1000);

//         display_menu("3:Temperature", "4:Watering Freq");
//         thread_sleep_for(1000);

//         display_menu("5:Water Level", "6:Sensor Values");
//         thread_sleep_for(1000);
//     }
// }

// void keypad_ISR() {
//     // Read the key immediately when the interrupt occurs
//     unsigned char keycode = Keypad_Data & Keypad_Data.mask();

//     // Convert the keycode to its corresponding character using the lookup table
    
//     key = lookupTable[keycode];
//     printf("%d" , key);
//     // Display the action based on the pressed key
//     switch (key) 
//     {
//         case 49:
//             printf("\nexecuting case 1\n");
//             lcd_Clear();
//             lcd_write_string("Grow Light");
//             printf("Action: Grow Light\n");
//             break;
//         case '2':
//             lcd_Clear();
//             lcd_write_string("Feeding Freq");
//             printf("Action: Feeding Frequency\n");
//             break;
//         case '3':
//             lcd_Clear();
//             lcd_write_string("Temperature");
//             printf("Action: Temperature\n");
//             break;
//         case '4':
//             lcd_Clear();
//             lcd_write_string("Watering Freq");
//             printf("Action: Watering Frequency\n");
//             break;
//         case '5':
//             lcd_Clear();
//             lcd_write_string("Water Level");
//             printf("Action: Water Level\n");
//             break;
//         case '6':
//             lcd_Clear();
//             lcd_write_string("Sensor Values");
//             printf("Action: Sensor Values\n");
//             break;
//         default:
//             lcd_Clear();
//             lcd_write_string("Invalid Input");
//             printf("Action: Invalid Input\n");
//             break;
//     }

//     // Pause for a moment before returning to the main menu
//     thread_sleep_for(2000);
// }
