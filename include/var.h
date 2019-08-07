#ifndef VAR_H
#define VAR_H

#define tim_clock 0x05

extern unsigned int start;
extern volatile unsigned char flag = 0x01, tmp; //{rx,update,t1mod,t3mod,t1_bell,t3_bell,change,auto}
extern volatile unsigned char flag1 = 0; //{*,*,*,*,*,t0_bell,t0,lamp}
extern volatile unsigned int* thi;
extern volatile unsigned int stu_time = 0, sto_time = 0, total = 0, thi_lst;
extern unsigned int stu = 0, sto = 0;

enum mod
{
    BELL = 0,
    WORK1 = 1, //show work time page 1
    WORK2 = 2, //show work time page 2
    WORK3 = 3,
    SET1 = 4,
    SET2 = 5, //set work time
    TC1 = 6, //timer
    TC3 = 7
};
extern unsigned char mode;

#endif