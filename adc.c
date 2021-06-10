#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"


void adc_init(void)
{
    // Initialize the ADC

    // Set analog range to 0-5V, set ADMUX REF bits to choose AVCC, REFS[7:6] = 01
    ADMUX &= ~(1<<REFS1); ADMUX |= (1<<REFS0); 
    // Set prescalar to 128 (reduces clock frequency): ADPS[2:0] = 111
    ADCSRA |= ((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0));
    // Set ADC to 8-bit accuracy, ADLAR = 1, 
    ADMUX |= (1<<ADLAR);
    // Enable ADC, ADEN = 1
    ADCSRA |= (1<<ADEN);


}

unsigned char adc_sample(unsigned char channel)
{
    // Convert an analog input and return the 8-bit result

	// Clear ADMUX MUX bits and then set to desired channel
	ADMUX &= ~15; ADMUX |= channel;
	// Begin conversion process, ADSC = 1
	ADCSRA |= (1<<ADSC);
	// Do nothing until conversion is complete, ADSC = 0
	while((ADCSRA & (1<<ADSC)) != 0){}

	return ADCH;
}
