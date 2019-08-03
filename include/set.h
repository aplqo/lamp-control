#ifndef SET_H
#define SET_H

enum type
{
    TIME = 0,
    INT = 1
};

void load(unsigned char pos, unsigned int* i, enum type typ);
void set_val(unsigned char val);

void add();
void dec();

void next_int();
void prev_int();

void next_time();
void prev_time();

#endif