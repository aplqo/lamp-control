#ifndef IR_H
#define IR_H

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
extern volatile enum ircode ir;

void ir_set();

#endif