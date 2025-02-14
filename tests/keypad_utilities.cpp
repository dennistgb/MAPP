/*
 * File:   keypad_utilities.cpp
 *
 */


#include <cstdio>
#undef __ARM_FP

#include "mbed.h"


//declare the Keypad Data input pins as an object of BusIn
//You can have up to 16 pins in a Bus.
//The order of pins in the constructor is the reverse order of the pins in the byte order. 
//If you have BusIn(a,b,c,d,e,f,g,h) 
//then the order of bits in the byte would be hgfedcba 
//with a being bit 0, b being bit 1, c being bit 2 and so on.
BusIn Keypad_Data(PB_8, PB_9, PB_10, PB_11);

DigitalIn Keypad_DA (PB_13); //74C922 DA output

const unsigned char lookupTable[] = {'1', '2','3','F','4','5','6','E','7','8','9','D','A','0','B','C'};

//----- Function to obtained wait for key press and returns its ASCII value

char getkey(void)
{	
    // Optional: set mode as PullUp/PullDown/PullNone/OpenDrain
    Keypad_Data.mode(PullNone);

    char keycode = 0;
     
    while (Keypad_DA == 0);      //wait for key to be pressed

    // read the bus and mask out bits not being used
    keycode = Keypad_Data & Keypad_Data.mask();    

    // output to COM port for debugging
    printf("Key pressed: 0x%04X \n", keycode);

    while (Keypad_DA == 1);      //wait for key to be released

    //Look up table to find the key pressed
    return (lookupTable[keycode]);
}

char getkey_nowait(void)
{
    // Optional: set mode as PullUp/PullDown/PullNone/OpenDrain
    Keypad_Data.mode(PullNone);
    printf("getting key press\n");

    char keycode = 0;
    Timer timer;

    timer.start();

    // Wait for key to be pressed or timeout
    while (Keypad_DA == 0) {
        if (timer.elapsed_time().count() > 5000000) { // 5 seconds timeout
            printf("No key pressed within 5 seconds.\n");
            timer.stop();
            return 0; // Return 0 or an appropriate value to indicate timeout
        }
    }

    // Stop the timer once the key is pressed
    timer.stop();
    timer.reset();

    // Read the bus and mask out bits not being used
    keycode = Keypad_Data & Keypad_Data.mask();

    // Output to COM port for debugging
    printf("Key pressed: 0x%04X \n", keycode);

    // Wait for key to be released
    while (Keypad_DA == 1);

    // Look up table to find the key pressed
    return (lookupTable[keycode]);
}





