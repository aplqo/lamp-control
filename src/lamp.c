#include "bit.h"
#include "ir.h"
#include "var.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#define t4_clock 0x08

struct status
{
    unsigned char Switch;
    unsigned char Light;
    unsigned char On; // if lamp is on
};
volatile struct status lamp;

void ir_lampt()
{
    switch (ir)
    {
    case START:
        flag1 |= 0x02;
        TCCR0B = tim_clock;
        break;
    case PAUSE:
        flag1 &= 0xfd;
        TCCR0B = 0x00;
        break;
    case STOP:
        TCCR0B = 0x00;
        stu_time = 0;
        sto_time = 0;
        total = 0;
        flag1 &= 0xfd;
        break;
    default:
        if (mode > WORK3)
        {
            ir_set();
        }
        else
        {
            return;
        }
        break;
    }
    flag &= 0x7f;
}

void ir_lamp()
{
    switch (ir)
    {
    case AUTO:
        flag = flag ^ 0x01;
        if (flag & 0x01)
        {
            led_auto(clr);
            int0(set);
        }
        else
        {
            led_auto(set);
            int0(clr);
        }
        break;
    case NEXT:
        if (lamp.Switch)
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
            bel(clr);
        }
        TCCR0B = 0x01;
        break;
    case SWITCH:
        if (flag & 0xfe)
        {
            flag1 = flag1 ^ 0x01;
            if (lamp.On & (flag1 & 0x01))
            {
                lampRelay(set);
                TCCR4B = t4_clock;
            }
            else
            {
                lampRelay(clr);
                TCCR4B = 0x00;
                TC4H = 0;
                TCNT4 = 0;
            }
        }
        break;
    default:
        break;
    }
    flag &= 0x7f;
}

ISR(INT0_vect)
{
    lamp.Light = (~lamp.Light);
    if ((lamp.Light && lamp.Switch) ^ lamp.On)
    {
        TCCR4B = t4_clock;
        lampRelay(set);
    }
    else
    {
        TCCR4B = 0x00;
        lampRelay(clr);
    }
}
ISR(INT1_vect)
{
    lamp.Switch = (~lamp.Switch);
    if (!(flag & 0x01))
        goto t;
    if ((lamp.Light && lamp.Switch) ^ lamp.On)
    {
        TCCR4B = t4_clock;
        lampRelay(set);
    }
    else
    {
        TCCR4B = 0x00;
        lampRelay(clr);
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
    lamp.On = (~lamp.On);
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
        lampRelay(set);
        TCCR4B = t4_clock;
    }
    else
    {
        lampRelay(clr);
        TCCR4B = 0x00;
    }
}
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
ISR(TIMER4_OVF_vect)
{
    static unsigned char i = 0;
    i++;
    if (i > start)
    {
        i = 0;
        asm("sbi %0,0x00" ::"i"(_SFR_IO_ADDR(PINF))
            :);
    }
}