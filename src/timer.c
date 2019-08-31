#include "bit.h"
#include "ir.h"
#include "var.h"
#include <avr/interrupt.h>
#include <avr/io.h>

volatile unsigned int t1_time = 0, t3_time = 0;

void ir_tc1()
{
    switch (ir)
    {
    case START:
        TCCR1B = tim_clock;
        break;
    case PAUSE:
        TCCR1B = 0x00;
        break;
    case STOP:
        TCCR1B = 0x00;
        TCNT1 = 0;
        t1_time = 0;
        flag &= 0xdf;
        break;
    default:
        ir_set();
        if (flag & 0x80)
        {
            return;
        }
        else
        {
            flag |= 0x20;
        }
        break;
    }
    flag &= 0x7f;
}
void ir_tc3()
{
    switch (ir)
    {
    case START:
        TCCR3B = tim_clock;
        break;
    case PAUSE:
        TCCR3B = 0x00;
        break;
    case STOP:
        TCCR3B = 0x00;
        TCNT3 = 0;
        t3_time = 0;
        flag &= 0xef;
        break;
    default:
        ir_set();
        if (flag & 0x80)
        {
            return;
        }
        else
        {
            flag |= 0x20;
        }
        break;
    }
    flag &= 0x7f;
}

ISR(TIMER1_OVF_vect)
{
    if (flag & 0x02)
    {
        t1_time--;
        if (t1_time == 0)
        {
            flag |= 0x4a;
            tmp = mode;
            mode = BELL;
            bel(set);
            return;
        }
    }
    else
    {
        t1_time++;
    }
    if (mode == TC1)
    {
        flag |= 0x40;
    }
}
ISR(TIMER3_OVF_vect)
{
    if (flag & 0x01)
    {
        t3_time--;
        if (t3_time == 0)
        {
            flag |= 0x46;
            tmp = mode;
            mode = BELL;
            bel(set);
            return;
        }
    }
    else
    {
        t3_time++;
    }
    if (mode == TC3)
    {
        flag |= 0x40;
    }
}