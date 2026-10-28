#include <avr/io.h> 
#include <util/delay.h> 
#include <LCD.h>

int main (void) 
{ 
  unsigned char i; 
  DDRB = 0xFF; 
  DDRD = 0xFF; 
  initlcd(); 
  lcd_puts("CE-LAB 2025"); 
  while (1){ 
    for (i = 0; i < 5; i++){ 
      LCD_putcmd(0x1C, LCD_2CYCLE); 
      _delay_ms(500); 
    } 
    for (i = 0; i < 5; i++){ 
      LCD_putcmd(0x18, LCD_2CYCLE); 
      _delay_ms(500); 
    } 
  } 
} 