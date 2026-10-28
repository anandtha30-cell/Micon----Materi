/*  05 Maret 2024 - Rudy S 
    Program untuk menyalakan dan mematikan LED di PORTD4 
    Untuk menunjukkan cara kerja timer, tapi tidak sebagai interrupt, hanya sebagai delay main program 
    Timer yang dipakai adalah Timer1, mode 16-bit 
    Case study:  
    User ingin toggle LED setiap 100ms 
    Hitungannya sbb: (F_CPU / Prescale) <-- dalam hal ini 16MHz/Pre-scale 
    kalau pakai 256 = 62,5KHz = 16uS setiap kali tambah angka (perubahan dari 0000 - 0001) 
    kalau pakai 1024 = 15,625 KHz setiap kali tambah angka  (perubahan dari 0000 - 0001) 
    nah kalau pakai 16-bit maka hanya pre-scale ini yang ada dibawah 65535 (angka max 16-bit) yang lain diatas angka ini 
    jadi pre-scale yang lain tidak bisa (1,8,64)  
    jadi kalau mau 100ms delay hitungannya: 
    100ms/16us = 6250 di register TCNT nya  
    100ms/64us = 1562 di register TCNT nya 
    kalau mau coba angka yang lain dipersilahkan.  
*/ 
#include <avr/io.h> 
#include <avr/interrupt.h>

#define LED PB5
int main() 
{
  DDRB |= (1<<LED) ; //LED as OUTPUT 
  TCCR1B = (1<<CS10) | (1<<CS12); //setting the pre-scalar as 1024  
  // TCCR1B = (1<<CS12); //set the pre-scalar as 256 
  OCR1A = 1562; //100ms delay 1562 dengan PS 1024 --> pakai ini kalau mau PS 1024 
  // OCR1A = 6250; //100ms delay 6250 dengan PS 256 --> pakai ini kalau mau PS 256 
  TCNT1 = 0; // <-- mulai hitung dari 0 
  while(1) 
  { 
    while((TIFR1 & (1<<OCF1A)) == 0);// wait till the timer compare flag is SET, tunggu status TIFR jadi 1. 
    PORTB ^= (1<< LED); // toggle PORTD4. 
    TCNT1 = 0; // reset TCNT. 
    TIFR1 |= (1<<OCF1A) ; // reset Timer 1 Flag. 
  }  
} 