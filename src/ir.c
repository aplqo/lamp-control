#include "bit.h"
#include "set.h"
#include "var.h"
#include <avr/interrupt.h>
#include <avr/io.h>

enum ircode
{
    POWER,
    PAGEUP,
    PAGEDW,
    /*timer code */
    START,
    PAUSE,
    STOP,
    /*set code */
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    /*lamp code */
    SWITCH,
    AUTO,
    NEXT
};
volatile enum ircode ir;

void ir_set()
{
    unsigned char t = 10;
    switch (ir)
    {
    case UP:
        add();
        break;
    case DOWN:
        dec();
        break;
    case LEFT:
        next();
        break;
    case RIGHT:
        prev();
        break;
    case ZERO:
        t = 0;
        break;
    case ONE:
        t = 1;
        break;
    case TWO:
        t = 2;
        break;
    case THREE:
        t = 3;
        break;
    case FOUR:
        t = 4;
        break;
    case FIVE:
        t = 5;
        break;
    case SIX:
        t = 6;
        break;
    case SEVEN:
        t = 7;
        break;
    case EIGHT:
        t = 8;
        break;
    case NINE:
        t = 9;
        break;
    default:
        return;
    }
    if (t < 10)
        set_val(t);
    flag &= 0x7f;
}

ISR(USART1_RX_vect)
{
    ir = UDR1;
    flag |= 0x80;
    led_rx(clr);
}