#include "ir.h"
#include "lamp.h"
#include "lcd.h"
#include "set.h"
#include "timer.h"
#include "var.h"
#include <avr/interrupt.h>
#include <avr/io.h>

void change()
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

void ir_mode()
{
    switch (ir)
    {
    case POWER:
        asm("sbi PINF,PINF1");
        return;
    case PAGEUP:
        mode++;
        if (mode > TC3)
        {
            mode = WORK1;
        }
        flag |= 0x02;
        lcd_write(0, 0x01);
        break;
    case PAGEDW:
        if (mode == WORK1)
        {
            mode = TC3;
        }
        flag |= 0x02;
        lcd_write(0, 0x01);
        break;
    default:
        return;
    }
    change();
    flag &= 0x7f;
}
void ir_bell()
{
    if (ir != START)
        return;
    flag &= 0xf3;
    flag1 &= 0xfb;
    asm("cbi PORTF,PORTF4");
    flag |= 0x02;
    mode = tmp;
    flag &= 0x7f;
}

void ir_ctrl()
{
    ir_mode();
    ir_lamp();
    if (flag & 0x80)
    {
        switch (mode)
        {
        case BELL:
            ir_bell();
            break;
        case TC1:
            ir_tc1();
            break;
        case TC3:
            ir_tc3();
            break;
        default:
            ir_lampt();
            break;
        }
    }
    flag &= 0x7f;
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