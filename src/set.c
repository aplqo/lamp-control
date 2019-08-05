#include "lcd.h"

enum type
{
    TIME = 0,
    INT = 1
};
struct typeConf
{
    unsigned char digit;
    unsigned char skip;
    unsigned char ratio[2];
};
const struct typeConf config[2] = {
    { 4, 3, { 10, 10 } },
    { 5, 1, { 6, 10 } }
};

struct
{
    unsigned char bit;
    unsigned char digit; //digit number
    unsigned int ratio;
    unsigned int* target;

    const struct typeConf* conf;
} set;

void update()
{
    lcd_write(1, set.bit);
    lcd_write(0, 0x10); //cursor left
}
void load(unsigned char pos, unsigned int* i, enum type typ)
{
    set.conf = config + typ;
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

void set_val(unsigned char val)
{
    if (val > set.bit)
    {
        *(set.target) += set.ratio * (val - set.bit);
    }
    else
    {
        *(set.target) -= set.ratio * (set.bit - val);
    }
    set.bit = val;
    update();
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

void next()
{
    if (set.digit == (set.conf->digit))
        return;
    unsigned char i = set.digit % 2;
    set.digit++;
    set.ratio *= (set.conf)->ratio[i];
    if (i == (set.conf)->skip)
    {
        lcd_write(0, 0x10);
    }
    lcd_write(0, 0x10);
}
void prev()
{
    if (set.digit == 0)
        return;
    unsigned char i = set.digit % 2;
    set.digit--;
    set.ratio /= (set.conf)->ratio[1 - i];
    if (i == ((set.conf)->skip) - 1)
    {
        lcd_write(0, 0x14);
    }
    lcd_write(0, 0x14);
}