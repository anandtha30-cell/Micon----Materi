#include <avr/io.h>
#include <avr/interrupt.h>

#define LED PB5
#define PRESCALER 256
#define F_CPU 16000000UL
#define FREQ_TRGT 1

int main() {
  DDRB |= (1 << LED);  // set PB5 as output

  // Timer1 normal mode + prescaler 256
  TCCR1A = 0;  
  TCCR1B = (1 << CS12);  // prescaler 256

  // Calculate preload
  uint16_t preload = 65536 - (F_CPU / (PRESCALER * FREQ_TRGT));

  TCNT1 = 3036;  // set initial preload

  while (1) {
    while ((TIFR1 & (1 << TOV1)) == 0);  // wait for overflow flag

    PORTB ^= (1 << LED);   // toggle LED
    TCNT1 = preload;       // reload preload value
    TIFR1 |= (1 << TOV1);  // clear overflow flag
  }
}
