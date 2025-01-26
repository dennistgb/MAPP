#include <cstdio>
#undef __ARM_FP
#include "mbed.h"
#include "DHT11.h"
#include <chrono>
using namespace std::chrono;

#define WAIT_TIME_MS_0 500
#define WAIT_TIME_MS_1 1500
#define ECHO_TIME_OUT 30ms  // Timeout duration for no object detection

DigitalOut led1(LED1);
DigitalIn button(PC_13);
DigitalOut Trig(PC_5);   // Trigger Pin
InterruptIn Echo(PC_6);  // Echo Pin with interrupt
Timer echoTimer;         // Timer to measure duration
Timeout echoTimeout;     // Timeout handler

#define DHT11_PIN PD_2
DHT11 dht11(DHT11_PIN);

float objDistance = 0;
bool bObjDetected = false;

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

// Function to get the temperature
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

// Function to get the humidity
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

int main()
{
    while (1)
    {
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
        thread_sleep_for(1000);
    }
}