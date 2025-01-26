#include <cstdio>
#undef __ARM_FP
#include "mbed.h"
#include "DHT11.h"

#define WAIT_TIME_MS_0 500
#define WAIT_TIME_MS_1 1500

DigitalOut led1(LED1);
DigitalIn button(PC_13);

#define DHT11_PIN PD_2
DHT11 dht11(DHT11_PIN);

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
int get_hum()
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
        return humidity; // Return the valid humidity value
    }
    else
    {
        printf("%s\n", dht11.getErrorString(humidity)); // Print the error
        return -1; // Return -1 to indicate an error
    }
}

int main()
{
    while(1)
    {
        int temperature = get_temp();
        if (temperature != -1)
        {
            printf("Temperature: %d °C\n", temperature);
        }
        else
        {
            printf("Failed to read temperature.\n");
        }

        int humidity = get_humidity();
        if (humidity != -1)
        {
            printf("Humidity: %d %%\n", humidity);
        }
        else
        {
            printf("Failed to read humidity.\n");
        }
    }
}

