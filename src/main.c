/*
port:
PB:lcd1602 D0~D7
PC7: led auto
PD0: light
PD1: switch
PD2: sirc receiver
PD3: lamp light
PE6: set
PF0: lamp control
PF1: power
PF4: bell
PF5: lcd1602 E
PF6: lcd1602 RS
PF7: lcd1602 RW
*/
#include "lcd.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#define tim_clock 0x05
#define t4_clock 0x80
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
volatile enum {
    POWER,
    PAGEUP,
    PAGEDW,
    START,
    PAUSE,
    UP,
    DOWN,
    SWITCH,
    STOP,
    LEFT,
    RIGHT,
    NUL,
    AUTO,
    NEXT
} ir;
struct
{
    unsigned char bi;
    unsigned char dig;
    unsigned int ratio;
    unsigned int* tar;
} set;
volatile struct
{
    unsigned char Switch;
    unsigned char Light;
    unsigned char On; // if lamp is on
} lamp;
unsigned char mode;
unsigned char start;
volatile unsigned char flag = 0x01, tmp; //{rx,update,t1mod,t3mod,t1_bell,t3_bell,change,auto}
volatile unsigned char flag1 = 0; //{*,*,*,*,*,t0_bell,t0,lamp}
unsigned char* thi;
volatile unsigned int stu_time = 0, sto_time = 0, total = 0, t1_time = 0, t3_time = 0, thi_lst;
unsigned int stu = 0, sto = 0;
/*-------function-------*/
/*---init---*/
inline void board_init()
{
    //init clock
    {
        CLKSEL0 |= 0x04;
        while (CLKSTA & 0x01)
            ;
        CLKSEL0 = 0x05;
    }
    //init gpio
    {
        DDRB = 0xff;
        DDRC = 0x80;
        DDRD = 0x00;
        DDRF = 0xf3;
        //PD pull up?
    }
    //init lcd1602
    lcd_write(0, 0x01); //clear
    lcd_write(0, 0x06); //left
    lcd_write(0, 0x0c); //display on
    lcd_write(0, 0x18); //2 row,8 bit bus,5x8
    //power save
    PRR0 = 0x85;
    PRR1 = 0x80;
}
void main_init()
{
    //read from eeprom
    {
        EEARL = 0x00;
        EECR = 0x01;
        start = EEDR;
    }
    //init usart
    {
        UCSR1C = 0x01;
        UCSR1B = 0x80;
        //Band Rate?
    }
    //init interrupt
    {
        EICRA = 0x45;
        EIMSK = 0x0b;
    }
    //init timer
    {
        TCCR0A = 0x02; //timer0 ctc mode
        OCR0A = 125;
        OCR0B = 62;
        TIMSK0 = 0x01;
        TCCR1A = 0x04;
        OCR1A = 15625;
        TIMSK1 = 0x01;
        TCCR3A = 0x04;
        OCR3A = 15625;
        TIMSK3 = 0x01;
        OCR4C = 125;
        TIMSK4 = 0x01;
    }
    //check the lamp
    {
        lamp.Light = (PIND & 0x01) ? 0xff : 0x00;
        lamp.On = (PIND & 0x08) ? 0xff : 0x00;
        lamp.Switch = (PIND & 0x02) ? 0xff : 0x00;
        if ((lamp.Light & lamp.Switch) ^ lamp.On)
        {
            asm("sbi PORTF,PORTF0");
            TCCR4B = t4_clock;
        }
    }
    asm("sei");
}
/*---ir control---*/
inline void ir_ctrl()
{
    switch (ir)
    {
    case POWER:
        asm("sbi PINF,PINF1");
        break;
    case PAGEUP:
        if (mode == BELL)
            break;
        mode++;
        if (mode > TC3)
        {
            mode = WORK1;
        }
        flag |= 0x02;
        lcd_write(0, 0x01);
        break;
    case PAGEDW:
        if (mode == BELL)
            break;
        if (mode == WORK1)
        {
            mode = TC3;
        }
        flag |= 0x02;
        lcd_write(0, 0x01);
        break;
    case START:
        switch (mode)
        {
        case TC1:
            TCCR1B = tim_clock;
            break;
        case TC3:
            TCCR3B = tim_clock;
            break;
        case BELL:
            flag &= 0xf3;
            flag1 &= 0xfb;
            asm("cbi PORTF,PORTF4");
            flag |= 0x02;
            mode = tmp;
            break;
        default:
            TCCR0B = tim_clock;
            break;
        }
        break;
    case PAUSE:
        switch (mode)
        {
        case TC1:
            TCCR1B = 0x00;
            break;
        case TC3:
            TCCR3B = 0x00;
            break;
        case BELL:
            break;
        default:
            TCCR0B = 0x00;
            break;
        }
        break;
    case UP:
        if (mode < SET1)
            break;
        if (set.bi < 9)
        {
            set.bi++;
            *(set.tar) = *(set.tar) + set.ratio;
        }
        else
        {
            set.bi = 0;
            *(set.tar) = *(set.tar) - set.ratio * 9;
        }
        lcd_write(1, set.bi);
        lcd_write(0, 0x10); //cursor left
        break;
    case DOWN:
        if (mode < SET1)
            break;
        if (set.bi == 0)
        {
            set.bi = 9;
            *(set.tar) = *(set.tar) + set.ratio * 9;
        }
        else
        {
            set.bi--;
            *(set.tar) = *(set.tar) - set.ratio;
        }
        lcd_write(1, set.bi);
        lcd_write(0, 0x10); //cursor left
        break;
    case SWITCH:
        if (flag & 0xfe)
        {
            flag1 = flag1 ^ 0x01;
            if (PORTD & (flag1 & 0x01))
            {
                asm("sbi PORTF,PORTF0");
                TCCR4B = t4_clock;
            }
            else
            {
                asm("cbi  PORTF,PORTF0");
                TCCR4B = 0x00;
                TC4H = 0;
                TCNT4 = 0;
            }
        }
        break;
    case STOP:
        switch (mode)
        {
        case TC1:
            TCCR1B = 0x00;
            t1_time = 0;
            TCNT1 = 0;
            break;
        case TC3:
            TCCR3B = 0x00;
            t3_time = 0;
            TCNT3 = 0;
            break;
        case BELL:
            break;
        default:
            TCCR0B = 0x00;
            stu_time = 0;
            sto_time = 0;
            total = 0;
            flag1 &= 0xfd;
            break;
        }
        break;
    case LEFT:
        if (set.dig == 8)
            break;
        if (set.dig % 2)
        {
            set.ratio *= 10;
            lcd_write(0, 0x10);
        }
        else
        {
            set.ratio *= 6;
        }
        set.dig++;
        lcd_write(0, 0x10);
        break;
    case RIGHT:
        if (set.dig == 0)
            break;
        if (set.dig % 2)
        {
            set.ratio /= 6;
        }
        else
        {
            set.ratio /= 10;
            lcd_write(0, 0x14);
        }
        set.dig++;
        lcd_write(0, 0x14); //cursor right
        break;
    case AUTO:
        flag = flag ^ 0x01;
        if (flag & 0x01)
        {
            asm("cbi PORTC,PORTC7");
            asm("sbi EIMSK,INT0");
        }
        else
        {
            asm("sbi PORTC,PORTC7");
            asm("cbi EIMSK,INT0");
        }
        break;
    case NEXT:
        if (PINB & 0x02)
        {
            thi_lst += stu;
        }
        else
        {
            thi_lst += sto;
        }
        flag1 &= 0xfb;
        if (!(flag & 0x0c))
        {
            asm("cbi PORTF,PORTF4");
        }
        TCCR0B = 0x01;
        break;
    default:
        break;
    }
    ir = NUL;
}
/*---update display---*/
inline void change()
{
    lcd_write(0, 0x01);
    lcd_write(0, 0x0c); //disable cursor
    switch (mode)
    {
    case TC1:
        display_str(0x00, 7, "Timer1:");
        set.tar = &t1_time;
        break;
    case TC3:
        display_str(0x00, 7, "Timer3:");
        set.tar = &t3_time;
        break;
    case SET1:
        display_str(0x00, 14, "Set work time:");
        set.tar = &stu;
        break;
    case SET2:
        display_str(0x00, 14, "Set rest time:");
        set.tar = &sto;
        break;
    case WORK1:
        display_str(0x00, 3, "Rem");
        display_str(0x40, 3, "Part");
        break;
    case WORK2:
        display_str(0x00, 4, "Work");
        display_str(0x40, 5, "Ratio");
        break;
    case WORK3:
        display_str(0x00, 4, "Rest");
        display_str(0x40, 3, "All");
        break;
    case BELL:
        display_str(0x00, 8, "Time up!");
        break;
    }
    if (mode > WORK3)
    {
        lcd_write(0, 0x0e); //open cursor
        set.bi = *(set.tar) % 10;
        set.dig = 1;
        set.ratio = 1;
        if (mode > SET2)
        {
            lcd_write(0, (0x80 | 0x0f));
        }
        else
        {
            lcd_write(0, (0x80 | 0x4f));
        }
    }
}
inline void refresh()
{
    unsigned char p = lcd_read() & 0x7f;
    lcd_write(0, 0x0c); //disable cursor
    switch (mode)
    {
    case WORK1:
        display_time(0x05, thi_lst);
        if (PIND & 0x02)
        {
            display_time(0x45, stu);
        }
        else
        {
            display_time(0x45, sto);
        }
        break;
    case WORK2:
        display_time(0x05, stu_time);
        {
            double rat = (double)stu_time / (double)total;
            display_double(0x46, rat);
        }
        break;
    case WORK3:
        display_time(0x05, sto_time);
        display_time(0x45, total);
        break;
    case TC1:
        display_time(0x45, t1_time);
        break;
    case TC3:
        display_time(0x45, t3_time);
        break;
    default:
        break;
    }
    lcd_write(0, (0x80 | p));
    lcd_write(0, 0x0e);
}
/*---interrupt---*/
ISR(TIMER0_OVF_vect)
{
    static unsigned char i = 0;
    i++;
    if (i == 125)
    {
        i = 0;
        (*thi)++;
        total++;
        if (thi_lst == 0)
        {
            flag1 |= 0x04;
            TCCR0B = 0x05;
        }
        else
        {
            thi_lst--;
        }
    }
}
ISR(TIMER0_COMPB_vect)
{
    static unsigned char i = 0;
    i++;
    if (i == 63)
    {
        asm("sbi PINF,PINF4");
        i = 0;
    }
}
ISR(TIMER1_OVF_vect)
{
    if (flag & 0x02)
    {
        t1_time--;
        if (t1_time == 0)
        {
            flag |= 0x4a;
            tmp = mode;
            mode = BELL;
            asm("sbi PORTF,PORTF4");
            return;
        }
    }
    else
    {
        t1_time++;
    }
    if (mode == TC1)
    {
        flag |= 0x40;
    }
}
ISR(TIMER3_OVF_vect)
{
    if (flag & 0x01)
    {
        t3_time--;
        if (t3_time == 0)
        {
            flag |= 0x46;
            tmp = mode;
            mode = BELL;
            asm("sbi PORTF,PORTF4");
            return;
        }
    }
    else
    {
        t3_time++;
    }
    if (mode == TC3)
    {
        flag |= 0x40;
    }
}
ISR(TIMER4_OVF_vect)
{
    static unsigned char i = 0;
    i++;
    if (i > start)
    {
        i = 0;
        asm("sbi PINF,PINF0");
    }
}
ISR(USART1_RX_vect)
{
    ir = UDR1;
    flag |= 0x80;
}
ISR(INT0_vect)
{
    ~lamp.Light;
    if ((lamp.Light && lamp.Switch) ^ lamp.On)
    {
        TCCR4B = t4_clock;
        asm("sbi PORTF,PORTF0");
    }
    else
    {
        TCCR4B = 0x00;
        asm("cbi PORTF,PORTF0");
    }
}
ISR(INT1_vect)
{
    ~lamp.Switch;
    if (!(flag & 0x01))
        goto t;
    if ((lamp.Light && lamp.Switch) ^ lamp.On)
    {
        TCCR4B = t4_clock;
        asm("sbi PORTF,PORTF0");
    }
    else
    {
        TCCR4B = 0x00;
        asm("cbi PORTF,PORTF0");
    }
t:;
    if (lamp.Switch)
    {
        thi = &stu_time;
    }
    else
    {
        thi = &sto_time;
    }
    TCNT0 = 0x00;
    flag |= 0x40;
}
ISR(INT3_vect)
{
    ~lamp.On;
    unsigned char expect;
    if (flag & 0x01)
    {
        expect = lamp.Light & lamp.On;
    }
    else
    {
        expect = (flag1 & 0x01) ? 0xff : 0x00;
    }
    if (expect ^ lamp.On)
    {
        asm("sbi PORTF,PORTF0");
        TCCR4B = t4_clock;
    }
    else
    {
        asm("cbi PORTF,PORTF0");
        TCCR4B = 0x00;
    }
}
/*---main---*/
int main(void)
{
    /* Insert system clock initialization code here (sysclk_init()). */

    board_init();

    main_init();

    /* Insert application code here, after the board has been initialized. */
    while (1)
    {
        if (ir != NUL)
        {
            ir_ctrl();
        }
        if (flag & 0x02)
        {
            change();
        }
        if (flag & 0x40)
        {
            refresh();
        }
    }
}
