#include <avr/io.h>
unsigned char lcd_read()
{
    unsigned char dat;
    DDRB = 0x00;
    PORTB = 0xff;
    asm("sbi PORTF,PORTF7");
    asm("nop");
    asm("sbi PORTF,PORTF5");
    dat = PINB;
    asm("cbi PORTF,PORTF5");
    asm("cbi PORTF,PORTF6");
    DDRB = 0xff;
    return dat;
}
void lcd_write(unsigned char rs, unsigned char dat)
{
    while (lcd_read() & 0x80)
        ;
    asm("cbi PORTF,PORTF7");
    if (rs)
    {
        asm("sbi PORTF,PORTF6");
    }
    PORTB = dat;
    asm("nop");
    asm("sbi PORTF,PORTF5");
    delay_us(150);
    asm("cbi PORTF,PORTF5");
    asm("nop");
    asm("cbi PORTF,PORTF6");
    PORTB = 0xff;
}