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
#include "ir.h"
#include "lcd.h"
#include "set.h"
#include "timer.h"
#include "var.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#define t4_clock 0x80
volatile struct
{
    unsigned char Switch;
    unsigned char Light;
    unsigned char On; // if lamp is on
} lamp;
/*-------function-------*/
/*---init---*/
inline void board_init()
{
    //init clock
    {
        CLKSEL0 |= 0x04;
        while (CLKSTA & 0x01)
            ;
        CLKSEL0 = 0x05;
    }
    //init gpio
    {
        DDRB = 0xff;
        DDRC = 0x80;
        DDRD = 0x00;
        DDRF = 0xf3;
        //PD pull up?
    }
    //init usart
    {
        UCSR1C = 0x01;
        UCSR1B = 0x80;
        //Band Rate?
    }
    //init lcd1602
    lcd_write(0, 0x01); //clear
    lcd_write(0, 0x06); //left
    lcd_write(0, 0x0c); //display on
    lcd_write(0, 0x18); //2 row,8 bit bus,5x8
    //power save
    PRR0 = 0x85;
    PRR1 = 0x80;
}
void main_init()
{
    //read from eeprom
    {
        EEARL = 0x00;
        EECR = 0x01;
        start = EEDR;
    }
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
        OCR4C = 125;
        TIMSK4 = 0x01;
    }
    //check the lamp
    {
        lamp.Light = (PIND & 0x01) ? 0xff : 0x00;
        lamp.On = (PIND & 0x08) ? 0xff : 0x00;
        lamp.Switch = (PIND & 0x02) ? 0xff : 0x00;
        if ((lamp.Light & lamp.Switch) ^ lamp.On)
        {
            asm("sbi PORTF,PORTF0");
            TCCR4B = t4_clock;
        }
    }
    asm("sei");
}
/*---ir control---*/
inline void ir_ctrl()
{
    switch (ir)
    {
    case POWER:
        asm("sbi PINF,PINF1");
        break;
    case PAGEUP:
        if (mode == BELL)
            break;
        mode++;
        if (mode > TC3)
        {
            mode = WORK1;
        }
        flag |= 0x02;
        lcd_write(0, 0x01);
        break;
    case PAGEDW:
        if (mode == BELL)
            break;
        if (mode == WORK1)
        {
            mode = TC3;
        }
        flag |= 0x02;
        lcd_write(0, 0x01);
        break;
    case START:
        switch (mode)
        {
        case BELL:
            flag &= 0xf3;
            flag1 &= 0xfb;
            asm("cbi PORTF,PORTF4");
            flag |= 0x02;
            mode = tmp;
            break;
        default:
            flag1 |= 0x02;
            TCCR0B = tim_clock;
            break;
        }
        break;
    case PAUSE:
        switch (mode)
        {
        case BELL:
            break;
        default:
            flag1 &= 0xfd;
            TCCR0B = 0x00;
            break;
        }
        break;
    case SWITCH:
        if (flag & 0xfe)
        {
            flag1 = flag1 ^ 0x01;
            if (PORTD & (flag1 & 0x01))
            {
                asm("sbi PORTF,PORTF0");
                TCCR4B = t4_clock;
            }
            else
            {
                asm("cbi  PORTF,PORTF0");
                TCCR4B = 0x00;
                TC4H = 0;
                TCNT4 = 0;
            }
        }
        break;
    case STOP:
        switch (mode)
        {
        case BELL:
            break;
        default:
            TCCR0B = 0x00;
            stu_time = 0;
            sto_time = 0;
            total = 0;
            flag1 &= 0xfd;
            break;
        }
        break;
    case AUTO:
        flag = flag ^ 0x01;
        if (flag & 0x01)
        {
            asm("cbi PORTC,PORTC7");
            asm("sbi EIMSK,INT0");
        }
        else
        {
            asm("sbi PORTC,PORTC7");
            asm("cbi EIMSK,INT0");
        }
        break;
    case NEXT:
        if (PINB & 0x02)
        {
            thi_lst += stu;
        }
        else
        {
            thi_lst += sto;
        }
        flag1 &= 0xfb;
        if (!(flag & 0x0c))
        {
            asm("cbi PORTF,PORTF4");
        }
        TCCR0B = 0x01;
        break;
    default:
        break;
    }
    flag &= 0x7f;
}
/*---update display---*/
inline void change()
{
    lcd_write(0, 0x01);
    lcd_write(0, 0x0c); //disable cursor
    unsigned int* i;
    switch (mode)
    {
    case TC1:
        display_str(0x00, 7, "Timer1:");
        i = &t1_time;
        break;
    case TC3:
        display_str(0x00, 7, "Timer3:");
        i = &t3_time;
        break;
    case SET1:
        display_str(0x00, 14, "Set work time:");
        i = &stu;
        break;
    case SET2:
        display_str(0x00, 14, "Set rest time:");
        i = &sto;
        break;
    case WORK1:
        display_str(0x00, 3, "Rem");
        display_str(0x40, 3, "Part");
        break;
    case WORK2:
        display_str(0x00, 4, "Work");
        display_str(0x40, 5, "Ratio");
        break;
    case WORK3:
        display_str(0x00, 4, "Rest");
        display_str(0x40, 3, "All");
        break;
    case BELL:
        display_str(0x00, 8, "Time up!");
        break;
    }
    if (mode > WORK3)
    {
        if (mode > SET2)
        {
            load(0x0f, i, TIME);
        }
        else
        {
            load(0x4f, i, TIME);
        }
    }
}
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
/*---interrupt---*/
ISR(TIMER0_OVF_vect)
{
    static unsigned char i = 0;
    i++;
    if (i == 125)
    {
        i = 0;
        (*thi)++;
        total++;
        if (thi_lst == 0)
        {
            flag1 |= 0x04;
            TCCR0B = 0x05;
        }
        else
        {
            thi_lst--;
        }
    }
}
ISR(TIMER0_COMPB_vect)
{
    static unsigned char i = 0;
    i++;
    if (i == 63)
    {
        asm("sbi PINF,PINF4");
        i = 0;
    }
}
ISR(TIMER4_OVF_vect)
{
    static unsigned char i = 0;
    i++;
    if (i > start)
    {
        i = 0;
        asm("sbi PINF,PINF0");
    }
}
ISR(INT0_vect)
{
    ~lamp.Light;
    if ((lamp.Light && lamp.Switch) ^ lamp.On)
    {
        TCCR4B = t4_clock;
        asm("sbi PORTF,PORTF0");
    }
    else
    {
        TCCR4B = 0x00;
        asm("cbi PORTF,PORTF0");
    }
}
ISR(INT1_vect)
{
    ~lamp.Switch;
    if (!(flag & 0x01))
        goto t;
    if ((lamp.Light && lamp.Switch) ^ lamp.On)
    {
        TCCR4B = t4_clock;
        asm("sbi PORTF,PORTF0");
    }
    else
    {
        TCCR4B = 0x00;
        asm("cbi PORTF,PORTF0");
    }
t:;
    if (flag1 & 0x02)
    {
        if (lamp.Switch)
        {
            thi = &stu_time;
            thi_lst = stu;
        }
        else
        {
            thi = &sto_time;
            thi_lst = sto;
        }
        TCNT0 = 0x00;
        flag |= 0x40;
    }
}
ISR(INT3_vect)
{
    ~lamp.On;
    unsigned char expect;
    if (flag & 0x01)
    {
        expect = lamp.Light & lamp.On;
    }
    else
    {
        expect = (flag1 & 0x01) ? 0xff : 0x00;
    }
    if (expect ^ lamp.On)
    {
        asm("sbi PORTF,PORTF0");
        TCCR4B = t4_clock;
    }
    else
    {
        asm("cbi PORTF,PORTF0");
        TCCR4B = 0x00;
    }
}
/*---main---*/
int main(void)
{
    /* Insert system clock initialization code here (sysclk_init()). */

    board_init();

    main_init();

    /* Insert application code here, after the board has been initialized. */
    while (1)
    {
        if (flag & 0x80)
        {
            ir_ctrl();
        }
        if (flag & 0x02)
        {
            change();
        }
        if (flag & 0x40)
        {
            refresh();
        }
    }
}
