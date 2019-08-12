#include "ir.h"
#include "lcd.h"
#include "set.h"
#include "var.h"
#include <avr/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define clock 0x0b
volatile unsigned int time = 0;

void measure()
{
    //init
    {
        TCCR1A = 0x00;
        OCR1A = 250;
        OCR1C = 0;
        TIMSK1 = 0x28;
        asm("sei");
    }
    display_str(0x00, 12, "Connect line");
    display_str(0x40, 14, "And close lamp");
    while (ir == START)
        ;
    lcd_write(0, 0x01);

    unsigned int t[2];
    for (unsigned char i = 0; i < 2; i++)
    {
        asm("sbi %0,0x00" ::"i"(_SFR_IO_ADDR(PORTF))
            :);
        TCCR1B = clock;
        while (TCCR1B)
            ;
        t[i] = time;
        asm("cbi %0,0x00" ::"i"(_SFR_IO_ADDR(PORTF))
            :);
        _delay_ms(50);
    }
    time = (t[0] > t[1]) ? t[0] : t[1];

    display_str(0x00, 11, "Delay time:");
    display_uint(0x40, time);
    load(0x40, &time, INT);

    while (ir != START)
    {
        if (flag & 0x80)
        {
            ir_set();
            continue;
        }
        if (ir == STOP)
        {
            lcd_write(0, 0x01);
            display_str(0x00, 11, "Press reset");
            while (1)
                ;
        }
    }
    lcd_write(0, 0x01);
    display_str(0x00, 7, "Writing");
    eeprom_write_word((uint16_t*)0x00, time);
    display_str(0x00, 11, "Press reset");
    while (1)
        ;
}
ISR(TIMER1_CAPT_vect)
{
    TCCR1B = 0x00;
}
ISR(TIMER1_COMPC_vect)
{
    time++;
}