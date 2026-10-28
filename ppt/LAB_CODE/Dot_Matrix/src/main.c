#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define COLUMN_D_MASK 0xFC
#define COLUMN_B_MASK 0x03
#define ROW_B_MASK    0x3C
#define ROW_C_MASK    0x0F
#define SCANS_PER_IMAGE 25

/*
	YOUR BITMAP PATTERN HERE
*/
static const uint8_t GLYPH_EMPTY[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t GLYPH_0[8]     = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_1[8]     = {0x18, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00};
static const uint8_t GLYPH_2[8]     = {0x3C, 0x66, 0x60, 0x30, 0x18, 0x0C, 0x7E, 0x00};
static const uint8_t GLYPH_3[8]     = {0x3C, 0x66, 0x60, 0x38, 0x60, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_4[8]     = {0x30, 0x38, 0x3C, 0x36, 0x7E, 0x30, 0x30, 0x00};
static const uint8_t GLYPH_5[8]     = {0x7E, 0x06, 0x3E, 0x60, 0x60, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_6[8]     = {0x3C, 0x66, 0x06, 0x3E, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_7[8]     = {0x7E, 0x66, 0x60, 0x30, 0x18, 0x18, 0x18, 0x00};
static const uint8_t GLYPH_8[8]     = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_9[8]     = {0x3C, 0x66, 0x66, 0x7C, 0x60, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_A[8]     = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00};
static const uint8_t GLYPH_B[8]     = {0x3E, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3E, 0x00};
static const uint8_t GLYPH_C[8]     = {0x3C, 0x66, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_D[8]     = {0x1E, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1E, 0x00};
static const uint8_t GLYPH_STAR[8]  = {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00};
static const uint8_t GLYPH_HASH[8]  = {0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00};

static const uint8_t* get_glyph_for_key(uint8_t key)
{
    switch (key) {
        case 0:  case '0': return GLYPH_0;
        case 1:  case '1': return GLYPH_1;
        case 2:  case '2': return GLYPH_2;
        case 3:  case '3': return GLYPH_3;
        case 4:  case '4': return GLYPH_4;
        case 5:  case '5': return GLYPH_5;
        case 6:  case '6': return GLYPH_6;
        case 7:  case '7': return GLYPH_7;
        case 8:  case '8': return GLYPH_8;
        case 9:  case '9': return GLYPH_9;
        case 10: case 'A': return GLYPH_A;
        case 11: case 'B': return GLYPH_B;
        case 12: case 'C': return GLYPH_C;
        case 13: case 'D': return GLYPH_D;
        case 14: case '*': return GLYPH_STAR;
        case 15: case '#': return GLYPH_HASH;
        default:           return GLYPH_EMPTY;
    }
}

static void uart_init(void)
{
    uint16_t ubrr = 103;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0B = (1 << RXEN0);                  // Aktifkan Receiver (RX)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bit, 1 stop bit, tanpa parity
}

/*
============================================================
8x8 LED MATRIX CONNECTION
============================================================

COLUMNS
--------
C0 -> PD2 -> Arduino D2
C1 -> PD3 -> Arduino D3
C2 -> PD4 -> Arduino D4
C3 -> PD5 -> Arduino D5
C4 -> PD6 -> Arduino D6
C5 -> PD7 -> Arduino D7
C6 -> PB0 -> Arduino D8
C7 -> PB1 -> Arduino D9

Each column:
AVR pin -> 330 ohm resistor -> LED anodes


ROWS
----
R0 -> PB2 -> Arduino D10
R1 -> PB3 -> Arduino D11
R2 -> PB4 -> Arduino D12
R3 -> PB5 -> Arduino D13
R4 -> PC0 -> Arduino A0
R5 -> PC1 -> Arduino A1
R6 -> PC2 -> Arduino A2
R7 -> PC3 -> Arduino A3

Each row:
AVR pin -> 1k resistor -> NPN base

NPN collector -> row LED cathodes
NPN emitter   -> GND


LOGIC
-----
Column HIGH = supplies current
Row HIGH    = turns NPN ON

Therefore:

pixel ON = selected column HIGH
           AND selected row transistor ON
============================================================
*/


/*
============================================================
BITMAP

Each byte represents one ROW.

bit 0 -> C0
bit 1 -> C1
...
bit 7 -> C7
============================================================
*/

/*
------------------------------------------------------------
Configure all GPIO used by the matrix
------------------------------------------------------------
*/
static void matrix_init(void)
{
    /*
     * PD2-PD7:
     * six column outputs
     */
    DDRD |=
        (1 << PD2) |
        (1 << PD3) |
        (1 << PD4) |
        (1 << PD5) |
        (1 << PD6) |
        (1 << PD7);

    /*
     * PB0-PB1:
     * two column outputs
     *
     * PB2-PB5:
     * four row transistor controls
     */
    DDRB |=
        (1 << PB0) |
        (1 << PB1) |
        (1 << PB2) |
        (1 << PB3) |
        (1 << PB4) |
        (1 << PB5);

    /*
     * PC0-PC3:
     * four row transistor controls
     */
    DDRC |=
        (1 << PC0) |
        (1 << PC1) |
        (1 << PC2) |
        (1 << PC3);

    /*
     * Initial safe condition:
     *
     * columns LOW
     * rows OFF
     */

    PORTD &= 0b00000011;

    PORTB &= ~(
        (1 << PB0) |
        (1 << PB1) |
        (1 << PB2) |
        (1 << PB3) |
        (1 << PB4) |
        (1 << PB5)
    );

    PORTC &= ~(
        (1 << PC0) |
        (1 << PC1) |
        (1 << PC2) |
        (1 << PC3)
    );
}

static void columns_write_bitmap(uint8_t bits)
{
 	PORTD = ((PORTD & ~COLUMN_D_MASK)
      | ((bits << 2) & COLUMN_D_MASK));
  	PORTB = ((PORTB & ~COLUMN_B_MASK)
      | ((bits >> 6) & COLUMN_B_MASK));
}

static void columns_write(uint8_t column)
{
	uint8_t bits = 0;
  	
  	if(column < 8) {
    	bits = (1 << column);
    }
  	columns_write_bitmap(bits);
}

static void row_on(uint8_t row)
{
  	if(row < 4) {
    		PORTB |= (1 << (row + 2));
  	} else if (row < 8) {
   		PORTC |= (1 << (row - 4)); 
  	}
}

static void rows_off(void)
{
    PORTB &= (uint8_t)~ROW_B_MASK;
    PORTC &= (uint8_t)~ROW_C_MASK;
}

static void row_slot(uint8_t row, uint8_t bits, uint8_t slow)
{
    rows_off();
    _delay_us(5);        
    columns_write_bitmap(bits);
    row_on(row);
    
    if (slow) { _delay_ms(250); }
    else      { _delay_ms(1);   }
    rows_off();
}

static void scan_image(const uint8_t image[8], uint8_t slow)
{
    for (uint8_t row=0; row<8; ++row) {
        row_slot(row, image[row], slow);
    }
}

static void hold_image(const uint8_t image[8], uint16_t scans)
{
    /* Holding content STILL means repeatedly scanning it.
     * There is no long delay after a scan. Animation therefore stays visible. */
    for (uint16_t n=0; n<scans; ++n) { scan_image(image,0); }
}


/*
============================================================
MAIN PROGRAM
============================================================
*/
int main(void)
{
    matrix_init();
    uart_init();
    const uint8_t* current_glyph = GLYPH_EMPTY;

    while (1)
    {
        if (UCSR0A & (1 << RXC0)) {
            char received_key = UDR0;
            current_glyph = get_glyph_for_key(received_key);
        }
        scan_image(current_glyph, 0);


    }

    return 0;
}
