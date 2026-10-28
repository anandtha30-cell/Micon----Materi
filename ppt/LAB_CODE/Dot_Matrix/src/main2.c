#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

static void uart_init(void)
{
    uint16_t ubrr = 103; // 9600 bps @ 16 MHz
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0B = (1 << TXEN0);                  // Aktifkan Transmitter (TX)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bit, 1 stop bit
}

static void uart_transmit(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0))); // Tunggu buffer TX kosong
    UDR0 = data;
}

/* Digits keep their values; A-D = 10-13, * = 14, # = 15. */
static const uint8_t key_code[4][4] = {
    { 1, 2, 3, 10 }, // 1 2 3 A
    { 4, 5, 6, 11 }, // 4 5 6 B
    { 7, 8, 9, 12 }, // 7 8 9 C
    {14, 0,15, 13 }  // * 0 # D
};

static int8_t keypad_scan_once(void)
{
    // Pastikan PC0..PC3 berlogika LOW saat menjadi output, dan mulai sebagai input
    PORTC &= (uint8_t)~0x0F;
    DDRC  &= (uint8_t)~0x0F;

    for (uint8_t row = 0; row < 4; row++) {
        // Jadikan HANYA baris yang dipilih sebagai output LOW (PC0..PC3 lainnya tetap input)
        DDRC = (DDRC & (uint8_t)~0x0F) | (1 << row);
        _delay_us(5);

        // Baca PD4..PD7 dari PIND
        uint8_t column = (PIND >> 4) & 0x0F;

        for (uint8_t col = 0; col < 4; col++) {
            if ((column & (1 << col)) == 0) {
                DDRC &= (uint8_t)~0x0F; // Wajib kembalikan semua baris jadi input sebelum return
                return key_code[row][col];
            }
        }
    }

    DDRC &= (uint8_t)~0x0F; // Kembalikan semua baris jadi input sebelum return
    return -1;
}

static void gpio_init(void)
{
    PORTC &= (uint8_t)~0x0F; // row input pull-ups off
    DDRC  &= (uint8_t)~0x0F; // rows initially inactive
    DDRD  &= (uint8_t)~0xF0; // PD4..PD7 inputs
    PORTD |= 0xF0;           // column pull-ups on
    DDRB  |= 0x0F;           // PB0..PB3 LED outputs
    PORTB &= (uint8_t)~0x0F; // LEDs initially off
}

static void keypad_task(void)
{
    int8_t first = keypad_scan_once();       // first reading
    if (first < 0) return;                   // -1 = no key; 0 is valid
    _delay_ms(20);                           // allow bounce to settle
    if (keypad_scan_once() != first) return; // reject a changed reading

    // Tampilkan ke LED PB0..PB3 dan kirim kode (0..15) via UART ke Arduino 1
    PORTB = (PORTB & 0xF0) | ((uint8_t)first & 0x0F);
    uart_transmit((uint8_t)first);

    // Wait until release is stable for 20 ms.
    do {
        while (keypad_scan_once() >= 0) { }
        _delay_ms(20);
    } while (keypad_scan_once() >= 0);
}

int main(void)
{
    gpio_init();
    uart_init();

    for (;;) {
        keypad_task();
    }
}
