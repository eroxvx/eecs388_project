#include <stdint.h>
#include <stdio.h>
#include "eecs388_lib.h"

#define Flashing_period 100

int main()
{
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);
    ser_setup();

    ser_printline("Setup completed.\n");

    uint16_t dist = 0;

    while (1) {

    if ('Y' == ser_read() && 'Y' == ser_read()) {
        dist = ser_read();
        dist |= ser_read() << 8;

        for(int i = 0; i < 5; i++) {
            ser_read();
        }

        if (dist > 200) {
            gpio_write(RED_LED, OFF);
            gpio_write(GREEN_LED, ON);
        } 

        else if (dist > 100) {
            gpio_write(RED_LED, ON);
            gpio_write(GREEN_LED, ON);
        }

        else if (dist > 60) {
            gpio_write(RED_LED, ON);
            gpio_write(GREEN_LED, OFF);
        }

        else {
            for (int i = 0; i < 10; i++) { 
                    gpio_write(RED_LED, ON);
                    delay(Flashing_period / 2);
                    gpio_write(RED_LED, OFF);
                    delay(Flashing_period / 2);
                }
        }

        printf("Measured distance: %d cm\n", dist);
    }
    }
}