unsigned char start;
volatile unsigned char flag = 0x01, tmp; //{rx,update,t1mod,t3mod,t1_bell,t3_bell,change,auto}
volatile unsigned char flag1 = 0; //{*,*,*,*,*,t0_bell,t0,lamp}
unsigned char* thi;
volatile unsigned int stu_time = 0, sto_time = 0, total = 0, t1_time = 0, t3_time = 0, thi_lst;
unsigned int stu = 0, sto = 0;