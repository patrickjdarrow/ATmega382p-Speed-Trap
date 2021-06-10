// Import global variables, common libraries, function declarations
#include "timers.h"

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