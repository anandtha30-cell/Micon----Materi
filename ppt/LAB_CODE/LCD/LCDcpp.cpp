#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

void LCD_setBg(bool state)
{
if(state)
 LCD_CONTROL |= (1<<BG);
else
 LCD_CONTROL &= ~(1<<BG);
}
void LCD_putch(unsigned char data)
{
LCD_CONTROL = (1<<EN)|(1<<RS)|(1<<BG);
LCD_PORT = (data&0xF0);
LCD_CONTROL &= ~(1<<EN);
_delay_us(1);
LCD_CONTROL = (1<<EN)|(1<<RS)|(1<<BG);
LCD_PORT = ((data&0x0F)<<4);
LCD_CONTROL &= ~(1<<EN);
_delay_ms(5);
}

void LCD_putcmd(unsigned char data, unsigned char cmdtype)
{
LCD_CONTROL = (1<<EN)|(1<<BG);
LCD_PORT = (data&0xF0);
LCD_CONTROL &= ~(1<<EN);
_delay_us(1);
if(cmdtype)
{
LCD_CONTROL = (1<<EN)|(1<<BG);
 LCD_PORT = ((data&0x0F)<<4);
 LCD_CONTROL &= ~(1<<EN);
}
_delay_ms(5);
} 

void initlcd()
{
LCD_setBg(true);
_delay_ms(30);
LCD_putcmd(0x30,LCD_1CYCLE);
_delay_ms(8);
LCD_putcmd(0x30,LCD_1CYCLE);
_delay_us(200);
LCD_putcmd(0x30,LCD_1CYCLE);
LCD_putcmd(0x20,LCD_1CYCLE);
LCD_putcmd(0x28,LCD_2CYCLE);
LCD_putcmd(0x0C,LCD_2CYCLE);
LCD_putcmd(0x01,LCD_2CYCLE);
LCD_putcmd(0x06,LCD_2CYCLE);
}

void lcd_puts(char *s)
{
while(*s != 0)
{
if(*s == '\n') LCD_putcmd(0xC0,LCD_2CYCLE);
 else LCD_putch(*s);
 s++;
}
}

void lcd_gotoxy (unsigned char x, unsigned char y)
{
unsigned char Position = (1 << 7);
if (y==1)
Position |= 0x40;
else if (y==2)
 Position |= 0x14;
else if (y==3)
 Position |= 0x54;
 Position += x;
LCD_putcmd(Position, LCD_2CYCLE);
}

void lcd_clear()
{
LCD_putcmd(0x01, LCD_2CYCLE);
} 

void lcd_putnum(int number)
{
 unsigned char digit;
 digit = '0';
 while(number >= 100){
 digit++;
 number -= 100;
 }
 if (digit != '0')
 LCD_putch(digit);
 else
 LCD_putch('0');
 digit = '0';
 while(number >= 10)
 {
 digit++;
 number -= 10;
 }
 if (digit != '0')
 LCD_putch(digit);
 else
 LCD_putch('0');
 LCD_putch('0' + number);
} 