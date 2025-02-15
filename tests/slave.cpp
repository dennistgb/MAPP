#include <cstdio>
#include <cstring>      // for strcmp
#undef __ARM_FP
#include "DHT11.h"
#include "mbed.h"
#include <string>


// For sleeping with chrono literals like 100ms, 3s, etc.
using namespace std::chrono_literals;

// -------------------------------
// Global Thresholds & Variables
// -------------------------------
#define WAIT_TIME_MS_0 500
#define WAIT_TIME_MS_1 1500
// Timeout duration for no object detection
#define ECHO_TIME_OUT 30ms
//Timer for fish feeder
#define INTERVAL 30
//definitions for fish feeder
#define WAIT_TIME_MS 1000          // Sleep time to allow motor to turn
#define PERIOD_WIDTH 20            // Period in ms according to the servo motor datasheet
#define PULSE_WIDTH_90_DEGREE 2400 // Pulse width in us to move to 90 degree position
#define PULSE_WIDTH_0_DEGREE 1500  // Pulse width in us to move to 0 position
#define PULSE_WIDTH_N_90_DEGREE 600 // Pulse width in us to move to -90 degree position
#define UART3_TX PC_10
#define UART3_RX PC_11

//Wifi
BufferedSerial serial_port(UART3_TX, UART3_RX, 115200);
char bufRx[512] = {0};
std::string localHost = "192.168.18.75"; // Change this to your server IP
std::string endpoint = "/update";          // Flask API endpoint

int led1 = 0;

//Warning system (RGB LED) pins
DigitalOut redPin(PB_1);      
DigitalOut greenPin(PB_3); 
DigitalOut bluePin(PB_4); 


// water level analog pin
AnalogIn water_level(PB_0); 
DigitalIn LDRSensor_DO(PD_1);

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

//define threshhold values
float TemperatureThreshhold = 28;
float DistanceThreshhold = 10;
float BrightnessThreshhold = 2.2;
float MoistThreshhold = 0.95;

DigitalOut GrowLight(PA_2);
DigitalOut relayWaterPump(PA_1);
DigitalOut fan(PA_10);
AnalogIn   LDRSensor_AO(PA_0);
AnalogIn   MoistureSensor(PA_4);

// UART pins TX=PB_6, RX=PB_7, 9600 baud
UnbufferedSerial uart(PB_6, PB_7, 9600);

// Character buffer
static constexpr size_t BUFFER_SIZE = 64;
char cmdArray[BUFFER_SIZE];
size_t cmdIndex = 0;
char buffer[3];
int idx = 0;
char controller_command[3] = {'a', 'a', '\0'};

//function to send AT command
// Function to send AT command and receive response
void sendATCommand(const std::string &cmd, int delay)
{
    serial_port.write(cmd.c_str(), cmd.length()); // Send command
    thread_sleep_for(delay);

    int bytesRead = serial_port.read(bufRx, sizeof(bufRx) - 1);
    if (bytesRead > 0)
    {
        bufRx[bytesRead] = '\0';
        //printf("%s\n", bufRx);
    }
}

// Function to connect to Wi-Fi
bool connectToWiFi(const std::string &ssid, const std::string &password)
{
    std::string command = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"\r\n";
    sendATCommand(command, 4500); // Increase delay for connection attempt

    // Check response for success
    if (strstr(bufRx, "WIFI CONNECTED") || strstr(bufRx, "OK"))
    {
        printf("Wi-Fi Connected Successfully!\n");
        return true;
    }
    else
    {
        printf("Wi-Fi Connection Failed!\n");
        return false;
    }
}

// Function to send sensor data to the Flask API
void sendHttpPost(float water, float moisture, float light, int temperature, int humidity, int lightstatus, int pump)
{
    // Flask API expects URL parameters
    std::string queryParams = "water_level=" + std::to_string(water) +
                              "&moisture_level=" + std::to_string(moisture) +
                              "&light_level=" + std::to_string(light) +
                              "&temperature=" + std::to_string(temperature) +
                              "&humidity=" + std::to_string(humidity) +
                              "&light_status=" + std::to_string(lightstatus) +
                              "&pump_status=" + std::to_string(pump);

    std::string requestUrl = endpoint + "?" + queryParams; // Full request URL
    std::string httpRequest =
        "POST " + requestUrl + " HTTP/1.1\r\n"
                               "Host: " +
        localHost + "\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";

    // Step 1: Open TCP connection
    std::string connectCommand = "AT+CIPSTART=\"TCP\",\"" + localHost + "\",3000\r\n";
    sendATCommand(connectCommand, 2000);

    // Step 2: Send data length
    std::string sendCommand = "AT+CIPSEND=" + std::to_string(httpRequest.length()) + "\r\n";
    sendATCommand(sendCommand, 1000);

    // Step 3: Send HTTP POST request
    sendATCommand(httpRequest, 1500);
}

//Change threshold values based on controller commands
void change_threshhold(const char *controller_command)
{
    printf("command is %s", controller_command);
    if (controller_command[0] == 'T') 
    {
        if (controller_command[1] == 'L') 
        {
            TemperatureThreshhold = 20;
        } 
        else 
        if (controller_command[1] == 'D') 
        {
            TemperatureThreshhold = 30;
        } 
        else if (controller_command[1] == 'H') 
        {
            TemperatureThreshhold = 40;
        } 
        else 
        {
            printf("Invalid Temperature Level\n");
        }
    }
    else if (controller_command[0] == 'D') 
    {
        if (controller_command[1] == 'L') 
        {
            DistanceThreshhold = 5;
        } 
        else if (controller_command[1] == 'M') 
        {
            DistanceThreshhold = 10;
        } 
        else if (controller_command[1] == 'H') 
        {
            DistanceThreshhold = 15;
        }
        else 
        {
            printf("Invalid Distance Level\n");
        }
    }
    else if (controller_command[0] == 'B') 
    {
        if (controller_command[1] == 'L') 
        {
            BrightnessThreshhold = 1.8;
        } 
        else if (controller_command[1] == 'D') 
        {
            BrightnessThreshhold = 2.2;            
        } 
        else if (controller_command[1] == 'H') 
        {
            BrightnessThreshhold = 3.0;
        } 
        else 
        {
            printf("Invalid Brightness Level\n");
        }
    }
    else if (controller_command[0] == 'M') 
    {
        if (controller_command[1] == 'L') 
        {
            MoistThreshhold = 0.8;
        } 
        else if (controller_command[1] == 'D') 
        {
            MoistThreshhold = 0.95;
        } 
        else if (controller_command[1] == 'H') 
        {
            MoistThreshhold = 1.2;
        } 
        else 
        {
            printf("Invalid Moisture Level\n");
        }
    }
    else 
    {
        printf("Action: Invalid Input\n");
    }
}

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
    relayWaterPump = 1;
    thread_sleep_for(2000);
    relayWaterPump = 0;
}

void rx_callback() {
    char c;
    if (uart.read(&c, 1) > 0) {
        if (idx == 0 && c == '\n') {
            return;  // Ignore '\n' as the first character
        }

        buffer[idx++] = c;

        if (c == '\n' || idx >= sizeof(buffer) - 1) {
            buffer[idx] = '\0';
            printf("Received: %s\n", buffer);
            for (int i = 0; i<3; i++){
            
                controller_command[i] = buffer[i];
                printf("controller_command is %c", controller_command[i]);
            }
            idx = 0;
        }
    }
}


void start_feeder()
{
    auto elapsed = feederTimer.elapsed_time();

    printf("servo spinning");
    motor.period_ms(PERIOD_WIDTH);                   // Set the period according to the servo specification
    motor.pulsewidth_us(PULSE_WIDTH_90_DEGREE);       // Move to 0 position (middle)
    printf("Elapsed time: %llu ms\n", chrono::duration_cast<chrono::milliseconds>(elapsed).count());
    feederTimer.start();  // Start the timer
    while (feederTimer.elapsed_time() < std::chrono::milliseconds(WAIT_TIME_MS)) // Wait until the time elapses
    {
        // Optionally, you can add other code here to do something during the waiting time
    }

    motor.pulsewidth_us(PULSE_WIDTH_0_DEGREE);      // Move to 90 degree (right)

    feederTimer.reset();  // Reset the timer for the next wait period
    feederTimer.start();  // Start the timer
    while (feederTimer.elapsed_time() < std::chrono::milliseconds(WAIT_TIME_MS)) // Wait until the time elapses
    {
        // Optionally, you can add other code here to do something during the waiting time
    }

    motor.pulsewidth_us(PULSE_WIDTH_90_DEGREE);    // Move to -90 degree (left)

    feederTimer.reset();  // Reset the timer for the next wait period
    feederTimer.start();  // Start the timer
    while (feederTimer.elapsed_time() < std::chrono::milliseconds(WAIT_TIME_MS)) // Wait until the time elapses
    {
        // Optionally, you can add other code here to do something during the waiting time
    }
    feederTimer.reset();  // Reset the timer for the next wait period

    motor.pulsewidth_us(0);      // Move to 90 degree (right)

    feederTimer.start();  // Start the timer
    while (feederTimer.elapsed_time() < std::chrono::milliseconds(WAIT_TIME_MS)) // Wait until the time elapses
    {
        // Optionally, you can add other code here to do something during the waiting time
    }
        feederTimer.reset();  // Reset the timer for the next wait period

}

int main()
{
    printf("Slave started...");
    LDRSensor_AO.set_reference_voltage(3.3f);
    feederTicker.attach(&start_feeder, INTERVAL);
    uart.attach(&rx_callback, SerialBase::RxIrq);
    serial_port.set_format(8, BufferedSerial::None, 1);
    serial_port.set_flow_control(BufferedSerial::Disabled);

    // Check ESP-01 status
    sendATCommand("AT\r\n", 1000);

    thread_sleep_for(500);

    // Attempt to connect to Wi-Fi
    if (connectToWiFi("Dickson", "4427216107"))
    {
        sendATCommand("AT+CIFSR\r\n", 1000); // Get IP address
    }
    else
    {
        printf("Retry connection or check credentials.\n");
    }

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
        int intDist = 0;
        if (distance != -1)
        {
            printf("Distance: %.2f cm\n", distance);
            if (distance > 10){
                intDist = 0;
            }
            else{
                intDist = 1;
            }
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
        printf("Communication phase\n");
        printf("command %s", controller_command);
        if (controller_command[0] != 'a') 
        {
            change_threshhold(controller_command);
        }
        sendHttpPost(Water_level, Moist, brightness, temperature,  humidity, led1, intDist);

        printf("Correctional phase\n");
        if (temperature > TemperatureThreshhold)
        {
            fan = 1;
        }
        else 
        {
            fan = 0;
        }
          
        //If water level too low 
        if(distance < DistanceThreshhold)
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
        if (brightness > BrightnessThreshhold)
        {
            GrowLight = 1;
        }
        else 
        {
            GrowLight = 0;
        }
        //If soil moisture too low pump water
        if (Moist < MoistThreshhold) 
        {
            pump_water();
        }
        printf("b threshold %f", TemperatureThreshhold);
    }
}