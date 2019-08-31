#include "bit.h"
#include <avr/io.h>
#include <util/delay.h>
/*---lcd driver--- */
static void pulse()
{
    lcd_en(clr);
    _delay_us(1);
    lcd_en(set);
    _delay_us(1);
    lcd_en(clr);
}
unsigned char lcd_read()
{
    unsigned char dat = 0x00;
    DDRB = 0x00;
    PORTB = 0x00;
    lcd_rw(set);
    asm("nop");

    lcd_en(set);
    asm("nop");
    dat = PINB & 0xf0;
    _delay_us(1);
    lcd_en(clr);
    _delay_us(100);

    lcd_en(set);
    asm("nop");
    dat |= PINB >> 4;
    _delay_us(1);
    lcd_en(clr);
    _delay_us(100);

    lcd_rs(clr);
    lcd_rw(clr);
    DDRB = 0xf0;
    return dat;
}
static void lcd_write4bit(unsigned char dat)
{
    PORTB = dat;
    pulse();
    _delay_us(100);
}
static void lcd_writeDirect(unsigned char rs, unsigned char dat) //write directly without wait for busy
{
    if (rs)
    {
        lcd_rs(set);
    }

    lcd_write4bit(dat & 0xf0); // write high
    lcd_write4bit(dat << 4); //write low

    lcd_rs(clr);

    PORTB = 0xf0;
}
void lcd_write(unsigned char rs, unsigned char dat)
{
    while (lcd_read() & 0x80)
        ;
    lcd_writeDirect(rs, dat);
}
void lcd_init()
{
    lcd_rs(clr);
    lcd_rw(clr);
    _delay_ms(15);

    lcd_write4bit(0x30);
    _delay_us(4500);

    lcd_write4bit(0x30);
    _delay_us(4500);

    lcd_write4bit(0x30);
    _delay_us(150);
    lcd_write4bit(0x20);
    lcd_writeDirect(0, 0x28);
    lcd_write(0, 0x06); // right
    lcd_write(0, 0x0c); // display on
    lcd_write(0, 0x01); // clear
}
/*---display---*/
void display_time(unsigned char pos, unsigned int dat)
{

    lcd_write(0, 0x80 | pos);
    unsigned char bit = 0;
    unsigned int ratio = 36000, t = dat;
    for (unsigned char i = 0; i < 6; i++)
    {
        bit = t / ratio;
        lcd_write(1, bit + 0x30);
        t = t % ratio;
        if (t % 2)
        {
            ratio = ratio / 10;
        }
        else
        {
            ratio = ratio / 6;
            lcd_write(1, ':');
        }
    }
}
void display_double(unsigned char pos, double dat)
{
    lcd_write(0, 0x80 | (pos + 8));
    lcd_write(0, 0x04);
    lcd_write(1, '%');
    unsigned long j = (unsigned long)(dat * 1000000);
    unsigned char bit = 0;
    for (unsigned char i = 0; i < 6; i++)
    {
        bit = j % 10;
        lcd_write(1, bit + 0x30);
        if (i == 3)
        {
            lcd_write(1, '.');
        }
        j = j / 10;
    }
    lcd_write(0, 0x06);
}
void display_str(unsigned char pos, unsigned char length, const char* str)
{
    lcd_write(0, (0x80 | pos));
    for (unsigned char i = 0; i < length; i++)
    {
        lcd_write(1, str[i]);
    }
}
void display_uint(unsigned char pos, unsigned int ui)
{
    lcd_write(0, 0x80 | (pos + 4));
    lcd_write(0, 0x06);
    unsigned char bit;
    for (unsigned char i = 0; i < 5; i++)
    {
        bit = pos % 10;
        lcd_write(1, bit + 0x30);
        pos /= 10;
    }
    lcd_write(0, 0x06);
}