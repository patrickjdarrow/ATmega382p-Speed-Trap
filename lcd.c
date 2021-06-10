/*
  lcd.c - Routines for sending data and commands to the LCD shield
*/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"                // Declarations of the LCD functions

/* This function not declared in lcd.h since
   should only be used by the routines in this file. */
void lcd_writenibble(unsigned char);

/* Define a couple of masks for the bits in Port B and Port D */
#define DATA_BITS ((1 << PD7)|(1 << PD6)|(1 << PD5)|(1 << PD4))
#define CTRL_BITS ((1 << PB1)|(1 << PB0))

/*
  lcd_init - Do various things to initialize the LCD display
*/
void lcd_init(void)
{
    /* ??? */                   // Set the DDR register bits for ports B and D
                                // Take care not to affect any unnecessary bits
  DDRD |= DATA_BITS;
  DDRB |= CTRL_BITS;


    _delay_ms(15);              // Delay at least 15ms

    lcd_writenibble(0x30);      // Use lcd_writenibble to send 0b0011
    _delay_ms(5);               // Delay at least 4msec

    lcd_writenibble(0x30);      // Use lcd_writenibble to send 0b0011
    _delay_us(120);             // Delay at least 100usec

    lcd_writenibble(0x30);      // Use lcd_writenibble to send 0b0011, no delay needed

    lcd_writenibble(0x20);      // Use lcd_writenibble to send 0b0010
    _delay_ms(2);               // Delay at least 2ms
    
    lcd_writecommand(0x28);     // Function Set: 4-bit interface, 2 lines

    lcd_writecommand(0x0f);     // Display and cursor on
}

/*
  lcd_moveto - Move the cursor to the row and column given by the arguments.
  Row is 0 or 1, column is 0 - 15.
*/
void lcd_moveto(unsigned char row, unsigned char col)
{
    unsigned char pos;
    if(row == 0) {
        pos = 0x80 + col;       // 1st row locations start at 0x80
    }
    else {
        pos = 0xc0 + col;       // 2nd row locations start at 0xc0
    }
    lcd_writecommand(pos);      // Send command
}

/*
  lcd_stringout - Print the contents of the character string "str"
  at the current cursor position.
*/
void lcd_stringout(char *str)
{
    int i = 0;
    while (str[i] != '\0') {    // Loop until next charater is NULL byte
        lcd_writedata(str[i]);  // Send the character
        i++;
    }
}

/*
  lcd_writecommand - Output a byte to the LCD command register.
*/
void lcd_writecommand(unsigned char cmd)
{
    /* Clear PB0 to 0 for a command transfer */
  PORTB &= ~(1 << PB0);

    /* Call lcd_writenibble to send UPPER four bits of "cmd" argument */
  lcd_writenibble(cmd);

    /* Call lcd_writenibble to send LOWER four bits of "cmd" argument */
  lcd_writenibble(cmd << 4);

    /* Delay 2ms */
  _delay_ms(2);

}

/*
  lcd_writedata - Output a byte to the LCD data register
*/
void lcd_writedata(unsigned char dat)
{
    /* Set PB0 to 1 for a data transfer */
  PORTB |= (1 << PB0);

    /* Call lcd_writenibble to send UPPER four bits of "cmd" argument */
  lcd_writenibble(dat);

    /* Call lcd_writenibble to send LOWER four bits of "cmd" argument */
  lcd_writenibble(dat << 4);
    /* Delay 2ms */
  _delay_ms(2);

}

/*
  lcd_writenibble - Output the UPPER four bits of "lcdbits" to the LCD
*/
void lcd_writenibble(unsigned char lcdbits)
{
    /* Load PORTD, bits 7-4 with bits 7-4 of "lcdbits" */

    /* Make E signal (PB1) go to 1 and back to 0 */
  PORTD &= (~DATA_BITS);            // Clear the top nibble of PORTD
  PORTD |= (DATA_BITS & lcdbits);    // Copy bits 7-4 of lcdbits to PORTD
  PORTB |= (1 << PB1);
  PORTB |= (1 << PB1);
  PORTB &= ~(1 << PB1);


}

// Make a pretty splash so the CPs like you
// Delays for ms_delay milliseconds or forever if ms_delay<=0
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