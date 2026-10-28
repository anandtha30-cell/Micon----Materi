#define LCD_CONTROL PORTB 
#define LCD_PORT PORTD
#define RS PB0 
#define EN PB1 
#define BG PB2 
#define LCD_1CYCLE 0 
#define LCD_2CYCLE 1 
void LCD_putch(unsigned char data); 
void LCD_putcmd(unsigned char data, unsigned char cmdtype); 
void initlcd(); 
void lcd_puts(char *s); 
void lcd_gotoxy(unsigned char x, unsigned char y); 
void lcd_clear(); 
void lcd_putnum(int number); 
void LCD_setBg(bool state); 