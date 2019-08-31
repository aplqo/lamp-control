/*
port:
PB:lcd1602 D0~D7
PC7: led auto
PD0: light
PD1: switch
PD2: sirc receiver
PD3: lamp light
PE6: set
PF0: lamp control
PF1: power
PF4: bell
PF5: lcd1602 E
PF6: lcd1602 RS
PF7: lcd1602 RW
*/
#include "bit.h"
#include "lamp.h"
#include "lcd.h"
#include "measure.h"
#include "mode.h"
#include "timer.h"
#include "var.h"
#include <avr/eeprom.h>
#include <avr/io.h>
/*-------function-------*/
/*---init---*/
inline void board_init()
{
    //init clock
    {
        CLKSEL0 |= 0x04;
        while (1)
        {
            if (CLKSTA & 0x01)
                break;
        }
        CLKSEL0 = 0x05;
        //init pll
        PLLFRQ = 0x04;
        PLLCSR = 0x13;
    }
    //init gpio
    {
        DDRB = 0xf0;
        DDRC = 0x80;
        DDRD = 0x10;
        DDRE = 0x00;
        DDRF = 0xf3;
        PORTD = 0x1f;
        PORTE = 0x40;
    }
    //init usart
    {
        UCSR1C = 0x01;
        UCSR1B = 0x80;
        UBRR1 = 103; // bandrate 9600
    }
    //init lcd1602
    lcd_init();
    //power save
    PRR0 = 0x85;
    PRR1 = 0x80;
}
void main_init()
{
    //read from eeprom
    start = eeprom_read_word((uint16_t*)0x00);
    //init interrupt
    {
        EICRA = 0x45;
        EIMSK = 0x0b;
    }
    //init timer
    {
        TCCR0A = 0x02; //timer0 ctc mode
        OCR0A = 125;
        OCR0B = 62;
        TIMSK0 = 0x01;
        TCCR1A = 0x04;
        OCR1A = 15625;
        TIMSK1 = 0x01;
        TCCR3A = 0x04;
        OCR3A = 15625;
        TIMSK3 = 0x01;
        TC4H = 0x01;
        OCR4C = 0x77;
        TIMSK4 = 0x01;
    }
    //check the lamp
    {
        lamp.Light = (PIND & 0x01) ? 0xff : 0x00;
        lamp.On = (PIND & 0x08) ? 0xff : 0x00;
        lamp.Switch = (PIND & 0x02) ? 0xff : 0x00;
        if ((lamp.Light & lamp.Switch) ^ lamp.On)
        {
            lampRelay(set);
            TCCR4B = t4_clock;
        }
    }
    asm("sei");
}
/*---update display---*/
inline void refresh()
{
    unsigned char p = lcd_read() & 0x7f;
    lcd_write(0, 0x0c); //disable cursor
    switch (mode)
    {
    case WORK1:
        display_time(0x05, thi_lst);
        if (PIND & 0x02)
        {
            display_time(0x45, stu);
        }
        else
        {
            display_time(0x45, sto);
        }
        break;
    case WORK2:
        display_time(0x05, stu_time);
        {
            double rat = (double)stu_time / (double)total;
            display_double(0x46, rat);
        }
        break;
    case WORK3:
        display_time(0x05, sto_time);
        display_time(0x45, total);
        break;
    case TC1:
        display_time(0x45, t1_time);
        break;
    case TC3:
        display_time(0x45, t3_time);
        break;
    default:
        break;
    }
    lcd_write(0, (0x80 | p));
    lcd_write(0, 0x0e);
}

/*---main---*/
int main(void)
{
    /* Insert system clock initialization code here (sysclk_init()). */

    board_init();
    if (PINE & 0x40)
    {
        measure();
    }

    main_init();
    change(); //init mode

    /* Insert application code here, after the board has been initialized. */
    while (1)
    {
        if (flag & 0x80)
        {
            ir_ctrl();
        }
        if (flag & 0x40)
        {
            refresh();
        }
    }
}
