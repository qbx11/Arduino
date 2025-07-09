#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
 
#define LED_BIT 6 //PD6
#define LED_PORT PORTD
#define LED_DDR DDRD

#ifndef _BV
#define _BV(bit)        (1<<(bit))
#endif
#ifndef sbi
#define sbi(reg,bit)    reg |= (_BV(bit))
#endif
 
#ifndef cbi
#define cbi(reg,bit)    reg &= ~(_BV(bit))
#endif

#ifndef tbi
#define tbi(reg,bit)    reg ^= (_BV(bit))
#endif

void pwm_init(){
  sbi(TCCR0A,WGM01);sbi(TCCR0A,WGM00); //fast PWM

  sbi(TCCR0A, COM0A1); //non-inverting mode (first 1, then 0)

  sbi(TCCR0B, CS01);   // preskaler = 64
  sbi(TCCR0B, CS00);   

  OCR0A =128;

}


int main() {
  pwm_init();
  //LED
  sbi(LED_DDR, LED_BIT);

  uint8_t b=0;

	while(1){
    OCR0A = b;//brightness
    b++;
    _delay_ms(50);
  }
}

