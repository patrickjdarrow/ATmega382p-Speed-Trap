#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "adc.h"


void init_timer0(unsigned short m);
void init_timer1(unsigned short m);
void serial_init(unsigned short);
void serial_stringout();
void serial_txchar(char);

void make_splash(char *l1, char *l2, int ms_delay, char side);
void write_int(int a, char r);
int write_time(int a);
void write_speed(int speed);
void write_speed_limit();
void write_char(char a, char c);

unsigned char bPressed();

void timer0_start_stop(int a);
void timer1_start_stop(int a);
void LED_start_stop(int a);
void rx_start_stop(int a);

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register
// ADC functions and variables
#define ADC_CHAN 0              // Buttons use ADC channel 0
// Timer constants
#define mc0 62500               // max count for timer0
#define mc1 62500               // max count for timer1 (4s per interval)
// Rotary encoder
#define PC_BITS ((1<<PC4) | (1<<PC5))


// FLAGS 
volatile unsigned char too_slow = 0;

// Loop index
unsigned char i;

// Rx flags
// tx/rx msg arrays
volatile char rxb[5];
volatile char rx_started = 0;
volatile char rx_finished = 0;
char txb[8];
// rx char index
volatile unsigned char rx_idx;

// Number of 0.25s intervals count1ed
volatile unsigned char count0, count1;

// Encoder vars
volatile unsigned char a, b;
volatile unsigned char input;
volatile unsigned char new_state, old_state, changed;
volatile int ecount;

// State encodings
volatile unsigned char state = 0;


int main(void) {
    // Grab EEPROM value
    ecount = eeprom_read_byte((void*)100);
    if( (ecount<0) | (ecount>99) ){ecount=0;}
    // Set IO pins for LED (PB5), buzzer (PB4), encoder (PC4, PC5)
    DDRB |= ((1<<PB5) | (1<<PB4));
    DDRC &= ~PC_BITS;
    PORTB &= ~ ((1<<PB5) | (1<<PB4));
    // Pullup resistors for PC4, PC5;
    PORTC |= PC_BITS;
    // Mask PORTC pin change interrupts for (PCINT9, PCINT13) = (A4, A5)
    PCMSK1 |= ((1<<PCINT12) | (1<<PCINT13));

    // Initialize the LCD, ADC and serial modules
    serial_init(MYUBRR);

    // Write a spash screen to the LCD
    // Initialize the LCD
    lcd_init();
    char *l1 = "Final Project";
    char *l2 = "Patrick Darrow";
    make_splash(l1, l2, 250, 1);
    l1 = "-- cm/s";
    l2 = "-.--- sec";
    make_splash(l1, l2, 0, 0);

    // Initialize the ADC
    adc_init();

    // Init timers
    init_timer0(mc0);
    init_timer1(mc1);

    // Speed/time variables
    unsigned int a1, a2, time_ms, speed;
    write_speed_limit();

    // Configure interrupts
    encoder_start_stop(1);
    rx_start_stop(1);
    LED_start_stop(0);
    UCSR0B |= (1<<RXCIE0);  // local enable
    sei();                  // global enable

    while (1) {
        // Sample the photoresistor pins
        a1 = adc_sample(2);
        a2 = adc_sample(3);

        // Display analog photoresistor values
        // write_int(a1, 0);
        // write_int(a2, 1);

        // timer disabled and LED off
        // if photoresistor1 is triggered:
        // begin timer and set state=1
        if(state==0){
            if(rx_finished){
                // rx_start_stop(0);
                // Store received message
                sscanf(rxb, "%04d", &speed);
                // Write out speed in cm/s
                speed /= 10;
                lcd_moveto(0,0);
                snprintf(rxb, 4, "%02d\0", speed);
                lcd_stringout(rxb);
                rx_finished = 0;
                // Beep for 1s if speed limit exceeded
                if(speed>ecount){timer0_start_stop(1);}
            }
            else if(a1<150){
                // Change state, display invalid time, reconfigure interrupts
                state=1;
                write_speed(-1);

                encoder_start_stop(0);
                LED_start_stop(1);
                timer1_start_stop(1);
                                rx_start_stop(0);

            }
            else if(changed){
                // If a rotary encoder state changes, adjust display
                changed = 0;
                eeprom_update_byte((void*)100, ecount);
                write_speed_limit();
            }
        }

        // Timer enabled and LED on
        // if photoresistor2 is triggered:
        // reset timer, display time, and set state=0
        if(state==1){
            time_ms = (count1 * 250) + (TCNT1 / 250);
            // Display time (ms)
            write_time(1);
            if( (a2<150) | too_slow){
                state=0;

                LED_start_stop(0);
                timer1_start_stop(0);
                encoder_start_stop(1);
                rx_start_stop(1);

                // If lap didn't complete in 4s, display invalid time/speed
                if(too_slow){
                    too_slow = 0;
                    write_time(-1);
                    write_speed(-1);
                }
                else{
                    // Store speed in mm/s
                    snprintf(txb, 7, "<%d>", 50000/time_ms);
                    // Transmit
                    serial_stringout();
                }
            }
        }

        // Debug delay
        // _delay_ms(400);
    }
}

/* ----------------------------------------------------------------------- */

ISR(PCINT1_vect){
        // For each state, examine the two input bits to see if state
        // has changed, and if so set "new_state" to the new state,
        // and adjust the count value.
        input = (PINC & PC_BITS);
        a = (input & (1<<PC4));
        b = (input & (1<<PC5));  
        if (old_state == 0) {
            if(a){new_state = 1; ecount++;}
            if(b){new_state = 2; ecount--;}
        }
        else if (old_state == 1) {
            if(!a){new_state = 0; ecount--;}
            if(b){new_state = 3; ecount++;}
        }
        else if (old_state == 2) {
            if(a){new_state = 3; ecount--;}
            if(!b){new_state = 0; ecount++;}
        }
        else {
            if(!a){new_state = 2; ecount++;}
            if(!b){new_state = 1; ecount--;}
        }

        if(ecount>99){ecount=0;}
        else if(ecount<0){ecount=99;}

        // If state changed, update the value of old_state,
        // and set a flag that the state has changed.
        if (new_state != old_state) {
            changed = 1;
            old_state = new_state;
        }

}

ISR(TIMER0_COMPA_vect){
    // Interrupt if 0.0016s (64us * 25) has passed
    // check if 1s of buzzing has passed
    count0++;
    // If 1 second of buzzing is over
    if(count0>=50){
        timer0_start_stop(0);
        count0 = 0; 
        PORTB &= ~(1<<PB4);
        return;
    }
    // else keep buzzing
    PORTB ^= (1<<PB4);
}

ISR(TIMER1_COMPA_vect){
    // Interrupt if 4s (0.25s * 16) has passed and flag the change
    count1++;
    if(count1>=16){
        too_slow = 1;
    }

}

ISR(USART_RX_vect){
    // Handle received character
    // while( !(UCSR0A & (1<<RXC0))){}
    char ch = UDR0;
    if(ch=='<'){
        rx_idx = 0;
        rx_finished = 0;
        rx_started = 1;
    }
    else if( (ch=='>') & (rx_idx>0)){rx_idx=0; rx_finished=1;}
    else if( rx_started & ( (ch>=48) & (ch<=57) ) ){
        rxb[rx_idx++] = ch;
    }
    else{
        rx_started = 0;
        for(i=rx_idx;i>=0;i--){
            rxb[i] = 0;
        }
    }
}

void init_timer0(unsigned short m){
    // Set to "Clear Timer on Compare (CTC)" mode
    TCCR0B |= (1<<WGM12);
    TCCR0B &= ~(1<<WGM13);

    // Enable Timer Interrupts
    TIMSK0 |= (1<<OCIE1A);

    // Load the MAX count
    OCR0A = 25;

    // Redundancy operation to ensure timer is stopped on initialization
    timer0_start_stop(0);
}

void init_timer1(unsigned short m){
    // Set to "Clear Timer on Compare (CTC)" mode
    TCCR1B |= (1<<WGM12);
    TCCR1B &= ~(1<<WGM13);

    // Enable Timer Interrupts
    TIMSK1 |= (1<<OCIE1A);

    // Load the MAX count
    OCR1A = m;

    // Redundancy operation to ensure timer is stopped on initialization
    timer1_start_stop(0);
}

void serial_init(unsigned short ubrr_value){
    // Enable pull-up resistors
    DDRD |= (1<<PD3);

    // Set baud rate = 9600
    UBRR0 = ubrr_value;

    // Enable RX/TX
    UCSR0C = (3<<UCSZ00);
    UCSR0B |= (1<<TXEN0 | 1<<RXEN0);

    // set PD3 high
    PORTD &= ~(1<<PD3);
}

void serial_txchar(char ch){
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}

void serial_stringout(){
    for(i=0; i<5; i++){
        serial_txchar(txb[i]);
    }
}

// Make a pretty splash so the CPs like you
void make_splash(char *l1, char *l2, int ms_delay, char side){
    lcd_writecommand(1);
    char lb[16];

    lcd_moveto(0, 0);
    if(side){snprintf(lb, 16, "%15s", l1);}
    else{snprintf(lb, 16, "%s", l1);}
    lcd_stringout(lb);

    lcd_moveto(1, 0);
    if(side){snprintf(lb, 16, "%15s", l2);}
    else{snprintf(lb, 16, "%s", l2);}
    lcd_stringout(lb);

    _delay_ms(ms_delay);

    if(ms_delay>0){
        lcd_writecommand(1);
    }
}

// Helper function for writing an int to a designated row
void write_int(int a, char r){
    lcd_writecommand(1);
    char ab[16];
    lcd_moveto(r, 0);
    snprintf(ab, 16, "%d", a);
    lcd_stringout(ab);
}

// Formats the value in TCNT1 to "seconds.milliseconds"
// returns time in ms
int write_time(int a){
    unsigned int time_ms = (count1 * 250) + (TCNT1 / 250);
    unsigned int b = count1 / 4;
    unsigned int c = time_ms % 1000;
    char ab[11];
    lcd_moveto(1, 0);
    // prints the whole seconds, followed by ms
    if(a==-1){snprintf(ab, 10, "-.--- sec");}
    else{snprintf(ab, 10, "%1d.%03d sec", b, c);}
    // snprintf(ab, 12, "%d", b);
    lcd_stringout(ab);
    return time_ms;
}

// void write_speed(unsigned time_ms){
//     char ab[9];
//     lcd_moveto(0, 0);
//     // prints the whole seconds, followed by ms
//     if(time_ms==-1){snprintf(ab, 8, "-- cm/s");}
//     else{snprintf(ab, 8, "%02d cm/s", 5000/time_ms);}
//     // snprintf(ab, 12, "%d", b);
//     lcd_stringout(ab);    
// }

// Write speed in cm/s
void write_speed(int speed){
    char ab[9];
    lcd_moveto(0, 0);
    // prints the whole seconds, followed by ms
    if(speed==-1){snprintf(ab, 8, "-- cm/s");}
    else{snprintf(ab, 8, "%02d cm/s", speed);}
    // snprintf(ab, 12, "%d", b);
    lcd_stringout(ab);    
    lcd_moveto(1, 15);
}

// Write char to a given position
void write_speed_limit()
{    
    char ab[8];
    lcd_moveto(0, 10);
    snprintf(ab, 7, "max:%02d", ecount);
    lcd_stringout(ab);
}

// Writes a char to a given position
void write_char(char a, char c){
    // in the center of the screen
    char ab[2];
    lcd_moveto(1, c);
    snprintf(ab, 2, "%s", a);
    lcd_stringout(ab);
}

void LED_start_stop(int a){
    if(a){
        PORTB |= (1<<PB5);
        return;
    }
    PORTB &= ~(1<<PB5);
}

// Resets counts (TCNT[0,1]) and starts or stops the timer
// a = start
// b = stop
void timer0_start_stop(int a){
    // Prescalar 1024
    count0 = 0;
    if(a){        
        TCNT0 = 0;
        TCCR0B &= ~(1<<CS01);
        TCCR0B |= (1<<CS02) | (1<<CS00);
        return;
    }
    TCCR0B &= ~( (1<<CS02) | (1<<CS01) | (1<<CS00) );
}
void timer1_start_stop(int a){
    // Prescalar 64
    if(a){        
        count1 = 0;
        TCNT1 = 0;
        TCCR1B &= ~(1<<CS12);
        TCCR1B |= (1<<CS11) | (1<<CS10);
        return;
    }
    TCCR1B &= ~( (1<<CS12) | (1<<CS11) | (1<<CS10) );
}

void encoder_start_stop(int a){
    if(a){
        // enable/disable pin change interrupts on PORTC
        PCICR |= (1<<PCIE1);
        // Read the A and B inputs to determine the intial state
        input = (PINC & PC_BITS);
        a = (input & (1<<PC4));
        b = (input & (1<<PC5));  
        if (!b && !a)
        old_state = 0;
        else if (!b && a)
        old_state = 1;
        else if (b && !a)
        old_state = 2;
        else
        old_state = 3;
        return;
    }
    PCICR &= ~(1<<PCIE1);
}

void rx_start_stop(int a){
    if(a){UCSR0B |= (1<<TXEN0 | 1<<RXEN0); return;}
    UCSR0B &= ~((1<<TXEN0) | (1<<RXEN0));
}