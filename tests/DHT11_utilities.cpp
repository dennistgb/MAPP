#include "DHT11.h"

char MSG_ERROR_TIMEOUT[] = "Error 253 Reading from DHT11 timed out.";
char MSG_ERROR_CHECKSUM[] = "Error 254 Checksum mismatch while reading from DHT11.";
char MSG_ERROR_UNKNOWN[] = "Error Unknown.";

/**
 * Constructor for the DHT11 class.
 * Initializes the pin to be used for communication and sets it to output mode.
 * @param pin: Pin Name on the STM32 NUCLEO-F103RB board to which the DHT11 sensor is connected.
 */
DHT11::DHT11(PinName pin) : _pin(pin)
{
    t.start(); // Start the timer
}

/**
 * Sets the delay between consecutive sensor readings.
 * If this method is not called, a default delay of 500 milliseconds is used.
 * @param delay: Delay duration in milliseconds between sensor readings.
 */
void DHT11::setDelay(unsigned long delay)
{
    _delayMS = delay;
}

/**
 * Reads raw data from the DHT11 sensor.
 * @param data: Array to store the raw data read from the sensor.
 * @return: Returns 0 if successful, or an error code if a timeout or checksum error occurs.
 */
int DHT11::readRawData(byte data[5])
{
    DigitalInOut pin_DHT11(_pin);
    pin_DHT11.output();
    pin_DHT11 = 1;
    thread_sleep_for(_delayMS);

    // Start signal
    pin_DHT11 = 0;
    thread_sleep_for(20);
    pin_DHT11 = 1;
    wait_us(30);
    pin_DHT11.input();

    t.reset(); // Reset timer
    int timeout_start = duration_cast<milliseconds>(t.elapsed_time()).count();

    // Wait for the response signal
    while (pin_DHT11 == 1)
    {
        if ((duration_cast<milliseconds>(t.elapsed_time()).count() - timeout_start) > TIMEOUT_DURATION)
            return DHT11::ERROR_TIMEOUT;
    }

    if (pin_DHT11 == 0)
    {
        wait_us(80); // Wait for the response
        if (pin_DHT11 == 1)
        {
            wait_us(80);
            for (int i = 0; i < 5; i++)
            {
                byte value = 0;

                // Read 8 bits
                for (int j = 0; j < 8; j++)
                {
                    while (pin_DHT11 == 0);
                    wait_us(30);
                    if (pin_DHT11 == 1)
                        value |= (1 << (7 - j));
                    while (pin_DHT11 == 1);
                }
                data[i] = value;
            }

            // Check checksum
            if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
                return 0; // Success
            else
                return DHT11::ERROR_CHECKSUM;
        }
    }
    return DHT11::ERROR_TIMEOUT;
}

int DHT11::readTemperature()
{
    byte data[5];
    int error = readRawData(data);
    if (error != 0)
        return error;
    return data[2]; // Temperature
}

int DHT11::readHumidity()
{
    byte data[5];
    int error = readRawData(data);
    if (error != 0)
        return error;
    return data[0]; // Humidity
}

int DHT11::readTemperatureHumidity(int &temperature, int &humidity)
{
    byte data[5];
    int error = readRawData(data);
    if (error != 0)
        return error;

    humidity = data[0];
    temperature = data[2];
    return 0; // Success
}

char* DHT11::getErrorString(int errorCode)
{
    switch (errorCode)
    {
        case DHT11::ERROR_TIMEOUT:
            return MSG_ERROR_TIMEOUT;
        case DHT11::ERROR_CHECKSUM:
            return MSG_ERROR_CHECKSUM;
        default:
            return MSG_ERROR_UNKNOWN;
    }
}
