
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

// Sensor
#define SENSOR1 PD2
#define SENSOR2 PD3

// Counter
#define COUNTER0 PC0
#define COUNTER1 PC1
#define COUNTER2 PC2
#define COUNTER3 PC3


// 7 Segment display
#define SEG_A PD4
#define SEG_B PD5
#define SEG_C PD6
#define SEG_D PD7
#define SEG_E PB0
#define SEG_F PB1
#define SEG_G PC4
#define DOT PC5

// 4 digit lamp
#define DIGIT1 PB2
#define DIGIT2 PB3
#define DIGIT3 PB4
#define DIGIT4 PB5



void check_sensors(void)
{
    uint32_t now;

    now = millis();


    //(PIND & (1 << SENSOR1))
    if (!(PIND & (1 << SENSOR1)))
    {
        if (!sensor1_state)
        {
            sensor1_state = 1;

            if (now - sensor1_time > DEBOUNCE_TIME)
            {
                sensor1_time = now;
            }
        }
    }
    else
    {
        sensor1_state = 0;
    }
    //(PIND & (1 << SENSOR2))
    if (!(PIND & (1 << SENSOR2)))
    {
        if (!sensor2_state)
        {
            sensor2_state = 1;

            if (now - sensor2_time > DEBOUNCE_TIME)
            {
                sensor2_time = now;
            }
        }
    }
    else
    {
        sensor2_state = 0;
    }
}


uint8_t vehicle_passed(void)
{
    uint32_t time_difference;


    check_sensors();

// eerst sensor 1
    if (sensor1_time == 0)
        return 0;

// dan sensor 2
    if (sensor2_time <= sensor1_time)
        return 0;


    time_difference = sensor2_time - sensor1_time;

// binnen tijd limit
    if (time_difference <= ACTIVATION_INTERVAL)
    {
        total_time = time_difference;

// reset
        sensor1_time = 0;
        sensor2_time = 0;

        return 1;
    }


    return 0;
}


main(){

    return 1
}