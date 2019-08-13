#include <avr/io.h>
#include <util/delay.h>
/*---io port operation---*/
#define set "sbi"
#define clr "sbi"
#define en(op) asm(op " %0,0x05" ::"i"(_SFR_IO_ADDR(PORTF)) \
                   :)
#define rw(op) asm(op " %0,0x07" ::"i"(_SFR_IO_ADDR(PORTF)) \
                   :)
#define rs(op) asm(op " %0,0x06" ::"i"(_SFR_IO_ADDR(PORTF)) \
                   :)
/*---lcd driver--- */
unsigned char lcd_read()
{
    en(clr);
    unsigned char dat;
    DDRB = 0x00;
    PORTB = 0xf0;
    rw(set);
    asm("nop");

    en(set);
    dat = PINB & 0xf0;
    en(clr);
    asm("nop");

    en(set);
    asm("nop");
    dat |= PINB >> 4;
    en(clr);

    rs(clr);
    DDRB = 0xf0;
    return dat;
}
void lcd_writeDirect(unsigned char rs, unsigned char dat) //write directly without wait for busy
{
    rw(clr);
    if (rs)
    {
        rs(set);
    }

    PORTB = dat & 0xf0; // write high
    asm("nop");
    en(set);
    asm("nop");
    en(clr);

    PORTB = dat << 4; //write low
    asm("nop");
    en(set);
    asm("nop");
    en(clr);

    asm("nop");
    rs(clr);

    PORTB = 0xf0;
}
void lcd_write(unsigned char rs, unsigned char dat)
{
    while (lcd_read() & 0x80)
        ;
    lcd_writeDirect(rs, dat);
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