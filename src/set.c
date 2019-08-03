#include "lcd.h"

enum type
{
    TIME = 0,
    INT = 1
};

struct
{
    unsigned char bit;
    unsigned char digit; //digit number
    unsigned int ratio;
    unsigned int* target;

    unsigned char pos;
    enum type t;
} set;

void update()
{
    lcd_write(1, set.bit);
    lcd_write(0, 0x10); //cursor left
}
void load(unsigned char pos, unsigned int* i, enum type typ)
{
    set.t = typ;
    set.pos = pos;
    set.digit = 0;
    set.target = i;
    set.ratio = 1;
    set.bit = (*i) % 10;

    switch (typ)
    {
    case TIME:
        pos += 7;
        break;
    case INT:
        pos += 4;
        break;
    }
    lcd_write(0, 0x80 + pos);
    lcd_write(0, 0x0e); // cursor on
}

void add()
{
    if (set.bit == 9)
        return;
    set.bit++;
    *(set.target) += set.ratio;
    update();
}
void dec()
{
    if (set.bit == 0)
    {
        return;
    }
    set.bit--;
    *(set.target) -= set.ratio;
    update();
}

void next_int()
{
    if (set.digit == 4)
        return;
    set.digit++;
    set.ratio *= 10;
    lcd_write(0, 0x10);
}
void prev_int()
{
    if (set.digit == 0)
        return;
    set.digit--;
    set.ratio *= 10;
    lcd_write(0, 0x14);
}

void next_time()
{
    if (set.digit == 5)
        return;
    set.digit++;
    if (set.digit % 2)
    {
        set.ratio *= 10;
    }
    else
    {
        set.ratio *= 6;
    }
    lcd_write(0, 0x10);
}
void prev_time()
{
    if (set.digit == 0)
        return;
    set.digit--;
    if (set.digit % 2)
    {
        set.ratio /= 6;
    }
    else
    {
        set.ratio /= 10;
    }
    lcd_write(0, 0x14);
}