
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



#define ACTIVATION_INTERVAL 8000UL
// Maximaal tijd voor oversteken voortuig

#define DEBOUNCE_TIME 100UL
// Tijd voor debounce 


volatile uint32_t milliseconds = 0;
//Volatile anders doet stoer 

uint32_t sensor1_time = 0;
uint32_t sensor2_time = 0;

uint8_t sensor1_state = 0;
uint8_t sensor2_state = 0;

uint32_t total_time = 0;

uint8_t counter = 0;
uint16_t speed = 0;


const uint8_t numbers[10] = {
    0b0111111,   //0
    0b0000110,   //1
    0b1011011,   //2
    0b1001111,   //3
    0b1100110,   //4
    0b1101101,   //5
    0b1111101,   //6
    0b0000111,   //7
    0b1111111,   //8
    0b1101111    //9
};


// Eigen millis functie voor interrupt elke 1ms

ISR(TIMER0_COMPA_vect)
{
    milliseconds++;
}

uint32_t millis(void)
{
    uint32_t time;

    cli();
    time = milliseconds;
    sei();

    return time;
}

//Set timer aan op CTC mode met ocra op 250 -1 zo is tijd ong 1ms + compare interrupt
void timer_setup(void)
{
    TCCR0A = (1 << WGM01);

    OCR0A = 249;

    //prescaler
    TCCR0B = (1 << CS01) | (1 << CS00);

    //compare interrupt
    TIMSK0 = (1 << OCIE0A);

    sei();
}




void io_setup(void)
{

    //input
    DDRD &= ~((1 << SENSOR1) | (1 << SENSOR2));

    //pull up
    PORTD |= (1 << SENSOR1) | (1 << SENSOR2);



    DDRC |=
    (1 << COUNTER0) |
    (1 << COUNTER1) |
    (1 << COUNTER2) |
    (1 << COUNTER3);



    DDRD |=
    (1 << SEG_A) |
    (1 << SEG_B) |
    (1 << SEG_C) |
    (1 << SEG_D);

    DDRB |=
    (1 << SEG_E) |
    (1 << SEG_F) |
    (1 << DIGIT1) |
    (1 << DIGIT2) |
    (1 << DIGIT3) |
    (1 << DIGIT4);

    DDRC |=
    (1 << SEG_G) |
    (1 << DOT);
}



void display_counter(uint8_t value)
{
    if (value & 1)
        PORTC |= (1 << COUNTER0);
    else
        PORTC &= ~(1 << COUNTER0);

    if (value & 2)
        PORTC |= (1 << COUNTER1);
    else
        PORTC &= ~(1 << COUNTER1);

    if (value & 4)
        PORTC |= (1 << COUNTER2);
    else
        PORTC &= ~(1 << COUNTER2);

    if (value & 8)
        PORTC |= (1 << COUNTER3);
    else
        PORTC &= ~(1 << COUNTER3);
}


void set_segments(uint8_t number)
{
    uint8_t pattern;

    pattern = numbers[number];

    if (pattern & (1 << 0))
        PORTD |= (1 << SEG_A);
    else
        PORTD &= ~(1 << SEG_A);


    if (pattern & (1 << 1))
        PORTD |= (1 << SEG_B);
    else
        PORTD &= ~(1 << SEG_B);


    if (pattern & (1 << 2))
        PORTD |= (1 << SEG_C);
    else
        PORTD &= ~(1 << SEG_C);


    if (pattern & (1 << 3))
        PORTD |= (1 << SEG_D);
    else
        PORTD &= ~(1 << SEG_D);


    if (pattern & (1 << 4))
        PORTB |= (1 << SEG_E);
    else
        PORTB &= ~(1 << SEG_E);


    if (pattern & (1 << 5))
        PORTB |= (1 << SEG_F);
    else
        PORTB &= ~(1 << SEG_F);


    if (pattern & (1 << 6))
        PORTC |= (1 << SEG_G);
    else
        PORTC &= ~(1 << SEG_G);
}

void digits_off(void)
{
    PORTB |=
    (1 << DIGIT1) |
    (1 << DIGIT2) |
    (1 << DIGIT3) |
    (1 << DIGIT4);
}



void show_digit(uint8_t number, uint8_t position)
{
    digits_off();

    set_segments(number);

    if (position == 0){
        PORTB &= ~(1 << DIGIT1);
    }
    if (position == 1){
        PORTB &= ~(1 << DIGIT2);
    }
    if (position == 2){
        PORTB &= ~(1 << DIGIT3);
    }
    if (position == 3){
        PORTB &= ~(1 << DIGIT4);
    }
    if (position == 2){
        PORTC |= (1 << DOT);
    } else {
        PORTC &= ~(1 << DOT);
    }    
    _delay_ms(5);
}

void display_speed(uint16_t value)
{
    uint8_t thousands;
    uint8_t hundreds;
    uint8_t tens;
    uint8_t ones;


    thousands = value / 1000;
    hundreds = (value / 100) % 10;
    tens = (value / 10) % 10;
    ones = value % 10;

    show_digit(thousands, 0);
    show_digit(hundreds, 1);
    show_digit(tens, 2);
    show_digit(ones, 3);
}

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

int main(void)
{
    io_setup();
    timer_setup();

    while (1)
    {
        if (vehicle_passed())
        {
            counter++;

            if (counter > 15)
                counter = 0;

            if (total_time > 0)
            {
                //60 cm distance -> speed in km/h × 10 */
                speed = 21600 / total_time;

                // min 0.2
                if (speed < 2)
                    speed = 2;
                // max 10
                if (speed > 100)
                    speed = 100;
            }
            else
            {
                speed = 2;
            }
        }

        display_counter(counter);

        display_speed(speed);
    }

    return 0;
}

// ik haat de 0 aan het einde
