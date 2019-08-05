#ifndef TIMER_H
#define TIMER_H

extern volatile unsigned int t1_time = 0, t3_time = 0;

void ir_tc1();
void ir_tc3();

#endif