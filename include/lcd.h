#ifndef LCD_H
#define LCD_H

unsigned char lcd_read();
void lcd_write(unsigned char rs, unsigned char dat);

/*---display--- */
void display_time(unsigned char pos, unsigned int dat);
void display_double(unsigned char pos, double dat);
void display_str(unsigned char pos, unsigned char length, const char* str);

#endif