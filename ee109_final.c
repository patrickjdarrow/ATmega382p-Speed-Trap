///////////////////////////////////////////////////////////
// Patrick Darrow EE109 - Friday 11AM Lab (the best lab) // 
///////////////////////////////////////////////////////////

// Library includes
#include "adc.h"
#include "encoder.h"
#include "lcd.h"
#include "led.h"
#include "serial.h"
#include "timers.h"
#include "util.h"

// Definitions
#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register
// ADC functions and variables
#define ADC_CHAN 0              // Buttons use ADC channel 0
// Timer constants
#define mc0 62500               // max count for timer0
#define mc1 62500               // max count for timer1 (4s per interval)
// Rotary encoder pins
#define PC_BITS ((1<<PC4) | (1<<PC5))

// State encodings
// 0: waiting for message rx or photoresistor state change
// 1: tracking time, waiting for photoresistor state change or timeout (4s)
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

        // Display analog photoresistor values (DEBUG)
        // write_int(a1, 0);
        // write_int(a2, 1);

        // timer disabled and LED off
        // if photoresistor1 is triggered:
        // begin timer and set state=1
        if(state==0){

            // If message is fully received, display it and beep if speed exceeds limit
            // else wait for message or state change
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
            // If speed trap is triggered
            // Change state, display invalid time, reconfigure interrupts
            else if(a1<150){
                state=1;
                write_speed(-1);

                encoder_start_stop(0);
                LED_start_stop(1);
                timer1_start_stop(1);
                rx_start_stop(0);

            }
            // If a rotary encoder state changes, adjust display
            else if(changed){
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

                // If lap didn't complete in 4s, display "invalid" time/speed
                if(too_slow){
                    too_slow = 0;
                    write_time(-1);
                    write_speed(-1);
                }
                // Else store speed in mm/s and transmit message
                else{
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
