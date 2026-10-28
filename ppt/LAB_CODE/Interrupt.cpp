#include <avr/io.h> 
#include <util/delay.h>
#include <avr/interrupt.h>

ISR(INT0_vect); 

int main(void){ 

  DDRD |= (1<<PD7); //set PD7 as output for LED 
  PORTD |= (1<<PD7); //turn on LED 
  DDRB |= (1<<PB5); //set PB5 as output for LED 

  // Set PD2 (INT0 pin) as input 
  DDRD &= ~(1 << PD2); 
  // Enable internal pull-up resistor for PD2 
  PORTD |= (1 << PD2); 
  
  // Disable global interrupt; 
  cli(); 

  // Configure external interrupt INT0 
  EICRA |= (1 << ISC01); // alternative: 0x01; falling Trigger interrupt on any logical change 
  EIMSK |= (1 << INT0); // alternative : 0x01; Enable external interrupt INT0 

  // Enable global interrupts 
  sei(); 

  while (1) //main 
  { 
    PORTB |= (1<<PB5); 
    _delay_ms(1000); 
    PORTB &= ~(1<<PB5); 
    _delay_ms(1000); 
  } 
  return 0; 
} 

// Interrupt service routine (ISR) for INT0 
ISR(INT0_vect){ 
  PORTD |= (1<<PD7); // the LED is on 
  _delay_ms (200); 
  PORTD &= ~(1<<PD7); // the LED is off 
  _delay_ms (150); 
} 