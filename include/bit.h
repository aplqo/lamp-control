#ifndef BIT_H
#define BIT_H

#include <avr/io.h>

/*-----operation definion-----*/
#define set "sbi"
#define clr "cbi"

/*-----bit definion-----*/
/*lcd port*/
#define lcd_en(op) asm(op " %0,0x05" ::"i"(_SFR_IO_ADDR(PORTF)) \
                       :)
#define lcd_rw(op) asm(op " %0,0x07" ::"i"(_SFR_IO_ADDR(PORTF)) \
                       :)
#define lcd_rs(op) asm(op " %0,0x06" ::"i"(_SFR_IO_ADDR(PORTF)) \
                       :)
/*control port*/
#define lampRelay(op) asm(op " %0,0x00" ::"i"(_SFR_IO_ADDR(PORTF)) \
                          :)
#define bel(op) asm(op " %0,0x04" ::"i"(_SFR_IO_ADDR(PORTF)) \
                    :)
#define power(op) asm(op " %0,0x01" ::"i"(_SFR_IO_ADDR(PORTF)) \
                      :)
/*led port*/
#define led_auto(op) asm(op " %0,0x07" ::"i"(_SFR_IO_ADDR(PORTC)) \
                         :)
/*interrupt enable*/
#define int0(op) asm(op " %0,0x00" ::"i"(_SFR_IO_ADDR(EIMSK)) \
                     :)
#endif