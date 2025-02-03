#include <cstdio>
#undef __ARM_FP
#include "mbed.h"
#include "DHT11.h"
#include <chrono>
using namespace std::chrono;

#define WAIT_TIME_MS_0 500
#define WAIT_TIME_MS_1 1500
// Timeout duration for no object detection
#define ECHO_TIME_OUT 30ms
//Timer for fish feeder
#define INTERVAL 10
//defitions for fish feeder
#define WAIT_TIME_MS 2000          // Sleep time to allow motor to turn
#define PERIOD_WIDTH 20            // Period in ms according to the servo motor datasheet
#define PULSE_WIDTH_90_DEGREE 2400 // Pulse width in us to move to 90 degree position
#define PULSE_WIDTH_0_DEGREE 1500  // Pulse width in us to move to 0 position
#define PULSE_WIDTH_N_90_DEGREE 600 // Pulse width in us to move to -90 degree position

//Grow light Pin
DigitalOut GrowLight(PA_2);

//Warning system (RGB LED) pins
DigitalOut redPin(PB_1);      
DigitalOut greenPin(PB_3); 
DigitalOut bluePin(PB_4); 

// moisture sensor analog pin 
AnalogIn MoistureSensor(PA_4);

//relay to control water pump
DigitalOut relay(PA_1);     

// water level analog pin
AnalogIn water_level(PB_0); 
DigitalIn LDRSensor_DO(PD_1);
AnalogIn LDRSensor_AO(PA_0);

//ultrasound pins
DigitalOut Trig(PC_5);   // Trigger Pin
InterruptIn Echo(PC_6);  // Echo Pin with interrupt
//timers for utlrasound
Timer echoTimer;         // Timer to measure duration
Timeout echoTimeout;     // Timeout handler

// servo motor pins for fish feeder
PwmOut motor(PA_7);
//ticker for fish feeder
Ticker feederTicker;
Timer feederTimer;

//dht pin
DHT11 dht11(PD_2);

float Moist = 0;
float Water_level = 0;
float objDistance = 0;
bool bObjDetected = false;
float LDR_AO_voltage = 0.0;

// ISR for Echo timeout
void ISR_EchoTimeout() 
{
    printf("Timeout: No obstacle detected!\n");
}
// ISR for rising edge (start of Echo signal)
void ISR_EchoHigh()
{
    echoTimer.reset();
    echoTimeout.attach(&ISR_EchoTimeout, ECHO_TIME_OUT);  // Attach timeout handler
}
// ISR for falling edge (end of Echo signal)
void ISR_EchoLow() 
{
    bObjDetected = true;
    echoTimeout.detach();  // Cancel timeout
    float duration = duration_cast<microseconds>(echoTimer.elapsed_time()).count();
    objDistance = (0.03432 * duration) / 2;  // Calculate distance in cm
}

// Function to measure distance using the ultrasonic sensor
float get_distance()
{
    // Send a 10us pulse to the Trigger pin
    Trig = 1;
    wait_us(10);
    Trig = 0;

    // Wait for Echo response
    echoTimer.start();
    while (!Echo.read())  // Wait for rising edge
    {
        if (echoTimer.elapsed_time() > ECHO_TIME_OUT)
        {
            echoTimer.stop();
            echoTimer.reset();
            return -1;  // Timeout, no object detected
        }
    }

    echoTimer.reset();
    echoTimer.start();
    while (Echo.read())  // Wait for falling edge
    {
        if (echoTimer.elapsed_time() > ECHO_TIME_OUT)
        {
            echoTimer.stop();
            echoTimer.reset();
            return -1;  // Timeout, no object detected
        }
    }

    echoTimer.stop();
    float duration = duration_cast<microseconds>(echoTimer.elapsed_time()).count();
    return (0.03432 * duration) / 2;  // Calculate distance in cm
}

// Function to get the temperature using DHT
int get_temp()
{
    int temperature = dht11.readTemperature();
    if (temperature != DHT11::ERROR_CHECKSUM && temperature != DHT11::ERROR_TIMEOUT)
    {
        return temperature; 
    }
    else
    {
        printf("%s\n", dht11.getErrorString(temperature)); 
        return -1; 
    }
}

// Function to get the humidity using DHT
int get_humidity()
{
    int humidity = dht11.readHumidity();
    if (humidity != DHT11::ERROR_CHECKSUM && humidity != DHT11::ERROR_TIMEOUT)
    {
        return humidity; 
    }
    else
    {
        printf("%s\n", dht11.getErrorString(humidity)); 
        return -1; 
    }
}

//Function to get brightness using LDR
int get_brightness()
{
    LDR_AO_voltage = LDRSensor_AO.read();
    LDR_AO_voltage = LDR_AO_voltage * 3.3;
    return LDR_AO_voltage;
}

void pump_water()
{
    relay = 1;
    thread_sleep_for(5000);
    relay = 0;
}

void start_feeder()
{
    motor.period_ms(PERIOD_WIDTH);                   // Set the period according to the servo specification
    motor.pulsewidth_us(PULSE_WIDTH_0_DEGREE);       // Move to 0 position (middle)
    printf("Move to 0 position: Middle\n");

    feederTimer.reset();  // Reset the timer
    feederTimer.start();  // Start the timer
    while (feederTimer.elapsed_time() < std::chrono::milliseconds(WAIT_TIME_MS)) // Wait until the time elapses
    {
        // Optionally, you can add other code here to do something during the waiting time
    }

    motor.pulsewidth_us(PULSE_WIDTH_90_DEGREE);      // Move to 90 degree (right)
    printf("Move to 90 degree position: Right\n");

    feederTimer.reset();  // Reset the timer for the next wait period
    feederTimer.start();  // Start the timer
    while (feederTimer.elapsed_time() < std::chrono::milliseconds(WAIT_TIME_MS)) // Wait until the time elapses
    {
        // Optionally, you can add other code here to do something during the waiting time
    }

    motor.pulsewidth_us(PULSE_WIDTH_N_90_DEGREE);    // Move to -90 degree (left)
    printf("Move to -90 degree position: Left\n");

    feederTimer.reset();  // Reset the timer for the next wait period
    feederTimer.start();  // Start the timer
    while (feederTimer.elapsed_time() < std::chrono::milliseconds(WAIT_TIME_MS)) // Wait until the time elapses
    {
        // Optionally, you can add other code here to do something during the waiting time
    }

    motor.period_ms(PERIOD_WIDTH);                   // Set the period according to the servo specification
}

int main()
{
    LDRSensor_AO.set_reference_voltage(3.3f);
    feederTicker.attach(&start_feeder, INTERVAL);
    
    while (1)
    {
        printf("Reading phase\n");
        // Temperature reading
        int temperature = get_temp();
        if (temperature != -1)
        {
            printf("Temperature: %d °C\n", temperature);
        }
        else
        {
            printf("Failed to read temperature.\n");
        }

        // Humidity reading
        int humidity = get_humidity();
        if (humidity != -1)
        {
            printf("Humidity: %d %%\n", humidity);
        }
        else
        {
            printf("Failed to read humidity.\n");
        }

        // Distance reading
        float distance = get_distance();
        if (distance != -1)
        {
            printf("Distance: %.2f cm\n", distance);
        }
        else
        {
            printf("Failed to detect distance.\n");
        }

        //brightness reading
        float brightness = get_brightness();
        printf("Brightness: %.3f V\n", brightness);

        Moist = MoistureSensor.read(); 
        printf("Soil Moisture: %.2f\n", Moist);
        
        Water_level = water_level.read(); 
        printf("Water Level: %.2f\n", Water_level);

        thread_sleep_for(1000);

        printf("Correctional phase\n");
        if (temperature == 50)
        {
            //start dc motor fan
        }
        if (humidity == 60)
        {
            //start dc motor fan
        }
        //If plant is too close (needs to be cut) put red on RGB LED
        if(distance == 10)
        {
            bluePin = 0;
            greenPin = 0;
            redPin = 1;
        }
        else 
        {
            bluePin = 0;
            greenPin = 1;
            redPin = 0;
        }
        //If brightness is too low turn on grow light
        if (brightness > 2.2)
        {
            GrowLight = 1;
        }
        else 
        {
            GrowLight = 0;
        }
        //If soil moisture too low pump water
        if (Moist == 0.95) 
        {
            pump_water();
        }
        //If water level to high, put up blue warning LED
        if (Water_level == 10)
        {
            bluePin = 1;
            greenPin = 0;
            redPin = 0;
        }
        else 
        {
            bluePin = 0;
            greenPin = 1;
            redPin = 0;
        }
    }
}