#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
 
#define LED_BIT 1 //PB0
#define LED_PORT PORTB
#define LED_DDR DDRB


#define PIR_BIT 0 //PB1
#define PIR_PIN PINB
#define PIR_DDR DDRB
#define PIR_PORT  PORTB

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

int main() {
  //led
  sbi(LED_DDR, LED_BIT);

  //PIR sensor (pull-up)
  cbi(PIR_DDR, PIR_BIT);
  sbi(PIR_PORT, PIR_BIT);

	while(1){
    if(bit_is_set(PIR_PIN,PIR_BIT))
    {
      sbi(LED_PORT, LED_BIT);
    }
    else{
      cbi(LED_PORT, LED_BIT);
    }
    _delay_ms(20);
     
  }
}
