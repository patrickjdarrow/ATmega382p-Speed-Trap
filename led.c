#include "led.h"

void LED_start_stop(int a){
    if(a){
        PORTB |= (1<<PB5);
        return;
    }
    PORTB &= ~(1<<PB5);
}
