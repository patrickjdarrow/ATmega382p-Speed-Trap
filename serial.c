// Import global variables, common libraries, function declarations
#include "serial.h"

// Waits for leading "<" to begin receipt
// Resets if non-numeric (0-9, ASCII)
// Finishes upon receiving "<" followed by some numerics, and ">"
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

// Helper functions for sending/receiving
void serial_txchar(char ch){
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}

void serial_stringout(){
    for(i=0; i<5; i++){
        serial_txchar(txb[i]);
    }
}

// Enable/disable rx/tx interrupts
void rx_start_stop(int a){
    if(a){UCSR0B |= (1<<TXEN0 | 1<<RXEN0); return;}
    UCSR0B &= ~((1<<TXEN0) | (1<<RXEN0));
}