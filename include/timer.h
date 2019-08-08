#ifndef TIMER_H
#define TIMER_H

extern volatile unsigned int t1_time, t3_time;

void ir_tc1();
void ir_tc3();

#endif