// Import global variables, common libraries, function declarations
#include "encoder.h"
// Rotary encoder
#define PC_BITS ((1<<PC4) | (1<<PC5))

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