// Import global variables, common libraries
#include "util.h"

void lcd_init(void);
void lcd_moveto(unsigned char, unsigned char);
void lcd_stringout(char *);
void lcd_writecommand(unsigned char);
void lcd_writedata(unsigned char);

// Quick functions for writing a variety of texts to the display
void make_splash(char *l1, char *l2, int ms_delay, char side);
void write_int(int a, char r);
int write_time(int a);
void write_speed(int speed);
void write_speed_limit();
void write_char(char a, char c);