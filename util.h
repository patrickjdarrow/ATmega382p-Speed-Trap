////////////////////////////////////////////////////////////////////
// All global vars/libs initialized here and included in .c files //
////////////////////////////////////////////////////////////////////

// Include common libraries to share across .h/.c files
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>

// Counting vars
volatile unsigned char count0, count1;
extern volatile unsigned char count0, count1;

volatile int ecount;
extern volatile int ecount;

// Loop index
volatile unsigned char i;
extern volatile unsigned char i;

// Rx flags
// tx/rx msg arrays
volatile char rxb[5];
extern volatile char rxb[5];

volatile char rx_started;
extern volatile char rx_started;

volatile char rx_finished;
extern volatile char rx_finished;

// rx char index
volatile unsigned char rx_idx;
extern volatile unsigned char rx_idx;

char txb[8];
extern char txb[8];

// Encoder vars
volatile unsigned char a, b;
extern volatile unsigned char a, b;

volatile unsigned char input;
extern volatile unsigned char input;

volatile unsigned char new_state, old_state, changed;
extern volatile unsigned char new_state, old_state, changed;

// Timer vars
unsigned char too_slow;
extern unsigned char too_slow;