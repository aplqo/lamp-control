#ifndef LAMP_H
#define LAMP_H
#define t4_clock 0x80

struct status
{
    unsigned char Switch;
    unsigned char Light;
    unsigned char On; // if lamp is on
};
extern volatile struct status lamp;

void ir_lamp();

#endif