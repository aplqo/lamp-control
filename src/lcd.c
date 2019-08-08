#include <avr/io.h>
#include <util/delay.h>
/*---lcd driver--- */
unsigned char lcd_read()
{
    unsigned char dat;
    DDRB = 0x00;
    PORTB = 0xff;
    asm("sbi %0,0x07" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    asm("nop");
    asm("sbi %0,0x05" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    dat = PINB;
    asm("cbi %0,0x05" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    asm("cbi %0,0x06" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    DDRB = 0xff;
    return dat;
}
void lcd_write(unsigned char rs, unsigned char dat)
{
    while (lcd_read() & 0x80)
        ;
    asm("sbi %0,0x07" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    if (rs)
    {
        asm("sbi %0,0x06" ::"i"(_SFR_IO_ADDR(PORTF))
            :);
    }
    PORTB = dat;
    asm("nop");
    asm("sbi %0,0x05" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    _delay_us(150);
    asm("cbi %0,0x05" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    asm("nop");
    asm("cbi %0,0x06" ::"i"(_SFR_IO_ADDR(PORTF))
        :);
    PORTB = 0xff;
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