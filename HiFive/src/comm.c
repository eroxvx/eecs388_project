#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "eecs388_lib.h"

#define FLASHING_PERIOD 100
#define SERVO_PULSE_MAX 2400 /* 2400 us */
#define SERVO_PULSE_MIN 544 /* 544 us */
#define SERVO_PERIOD 20000 /* 20000 us (20ms) */

uint16_t dist = 0;

void auto_brake(int devid)
{
    // Task-1: 
    // Your code here (Use Lab 02 - Lab 04 for reference)
    // Use the directions given in the project document
    
    // TFmini lidar
    // The sensor's data is encoded in a 9-byte data frame. THe first two two bytes are frame headers.
    // The next two bytes encode the actual distance data (cm) which are the low and high byte of the 16 bit distance data.
    // The remaining bits are not used thus are 'emptied' by reading five times.

    while(1){ // Ensure auto_brake occurs since steering might cause timing issues.
        if ('Y' == ser_read(devid) && 'Y' == ser_read(devid)) { // Check the header frames.

            dist = ser_read(devid); // Get the low byte of the distance data.
            dist |= ser_read(devid) << 8; // Get the high byte of the distance data and shift the bits the left.

            //printf("\ndistance=%d", dist); // Debugging purposes

            // Get the remaining data and 'empty' it.
            for(int i = 0; i < 5; i++) { 
                ser_read(devid);
            }

            if (dist > 200) {
                gpio_write(RED_LED, OFF);
                gpio_write(GREEN_LED, ON);
            } else if (dist > 100) {
                gpio_write(RED_LED, ON);
                gpio_write(GREEN_LED, ON);
            } else if (dist > 60) {
                gpio_write(GREEN_LED, OFF);
                gpio_write(RED_LED, ON);
            } else {
                gpio_write(GREEN_LED, OFF);
                gpio_write(RED_LED, ON);
                delay(FLASHING_PERIOD / 2);
                gpio_write(RED_LED, OFF);
                delay(FLASHING_PERIOD / 2);
            }    
            return;
        }
    }
}


int read_from_pi(int devid)
{
    // Task-2:
    // You code goes here (Use Lab 09 for reference)
    // After performing Task-2 at dnn.py code, modify this part to read angle values from Raspberry Pi.

    char buffer[8]; // A character array of size 8 used to store data read from the serial port.
    int angle, read_data; // Angle stores the converted integer read from the buffer, 
                          // and read_data stores the number of bytes read from the serial port.
    if(ser_isready(devid)){ // Check if the serial port is ready.
        read_data = ser_readline(devid, sizeof(buffer), buffer); // Read data from the serial port given the 
                                                                 // device id, buffer size, and buffer.
        if(read_data > 0) {
            sscanf(buffer, "%d", &angle); // Parses data from buffer and stores it in angle.
            printf("\nFrom PI to HIFIVE: read(%d) => %d \n", read_data, angle);
            return angle; // Returns the angle.          
        }else{
            return -1;
        }
    }
}

void steering(int gpio, int pos)
{
    // Task-3: 
    // Your code goes here (Use Lab 05 for reference)
    // Check the project document to understand the task

    // Pulse Width Modulation
    // RC servo motor operates betweens 544us (0 degrees) to 2400us (180 degrees) at a 20ms period.
    // PWM is a way to encode analog signals using digital outputs, 
    // and by altering the durations of ON and OFF we can encode an arbitrary analog signal.
    if(pos > 180 || pos < 0){
        return;
    }
    int pulse_width = ((SERVO_PULSE_MAX - SERVO_PULSE_MIN) * (pos/180.0)) + SERVO_PULSE_MIN;
    //printf("\npulse width=%d", pulse_width); // Debugging purposes
    gpio_write(gpio, ON);
    delay_usec(pulse_width);
    gpio_write(gpio, OFF);
    delay_usec(SERVO_PERIOD - pulse_width);
    //printf("\npulse width mod=%d", SERVO_PERIOD - pulse_width); // Debugging purposes
}


int main()
{
    // initialize UART channels
    ser_setup(0); // uart0
    ser_setup(1); // uart1
    int pi_to_hifive = 1; //The connection with Pi uses uart 1
    int lidar_to_hifive = 0; //the lidar uses uart 0
    
    printf("\nUsing UART %d for Pi -> HiFive", pi_to_hifive);
    printf("\nUsing UART %d for Lidar -> HiFive", lidar_to_hifive);
    
    //Initializing PINs
    gpio_mode(PIN_19, OUTPUT);
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(BLUE_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);

    printf("Setup completed.\n");
    printf("Begin the main loop.\n");

    while (1) {
        auto_brake(lidar_to_hifive); // measuring distance using lidar and braking
        int angle = read_from_pi(pi_to_hifive); //getting turn direction from pi
        printf("\nangle=%d", angle) 
        int gpio = PIN_19; 
        for (int i = 0; i < 10; i++){
            // Here, we set the angle to 180 if the prediction from the DNN is a positive angle
            // and 0 if the prediction is a negative angle.
            // This is so that it is easier to see the movement of the servo.
            // You are welcome to pass the angle values directly to the steering function.
            // If the servo function is written correctly, it should still work,
            // only the movements of the servo will be more subtle

            /*if(angle>0){
                steering(gpio, 180);
            }
            else {
                steering(gpio,0);
            }*/
            
            // Uncomment the line below to see the actual angles on the servo.
            // Remember to comment out the if-else statement above!
            steering(gpio, angle);
        }

    }
    return 0;
}