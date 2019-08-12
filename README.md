# lamp-control   
Auto control lamp and timer.
## Board
**mcu**: atmega32u4  
**board**: arduino yun (seeeduino cloud)  
**freq**: 16MHz  
## port connect
port(mcu)|port(board)|mode|use function|connect to|description|note
---------|-----------|----|------------|----------|-----------|----
PB4|D8|input/output|Digital I/O|LCD1602 D4|LCD data|
PB5|D9|input/output|Digital I/O|LCD1602 D5|LCD data|
PB6|D10|input/output|Digital I/O|LCD1602 D6|LCD data|
PB7|D11|input/output|Digital I/O|LCD1602 D7|LCD data|
PC7|D13|output|Digital I/O|LED auto|LED auto|LED is on board
PD0|D3|input|INT0|light sensor|bright check|Use digital seneor
PD1|D2|input|INT1|Human infared sensor|||
PD2|D0|input|USART RXD|IR decoder|Receive IR code|
PD3|D1|input|INT3|light sensor on lamp|check lamp|Use digital seneor
PD4|D4|input|ICP1|light sensor on lamp|Measuring delay|Connect to PD3
PE6|D7|input|Digital I/O|Nothing|Mode select|
PF0|A5|output|Digital I/O|Relay for controling lamp|||Connect NO;in:high
PF1|A4|output|Digital I/O|Relay for power||Connect NO;in:high
PF4|A3|output|Digital I/O|Bell||
PF5|A2|output|Digital I/O|LCD1602 E|LCD enable|
PF6|A1|output|Digital I/O|LCD1602 RS|LCD rs|
PF7|A0|output|Digital I/O|LCD1602 RW|LCD rw|