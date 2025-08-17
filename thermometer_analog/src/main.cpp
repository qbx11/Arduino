//LM35
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
 
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

void ADC_init(){
  //-------------//
  //reference voltage - maximal voltage value 
  sbi(ADMUX, REFS0); //AVcc (5V)

  //cbi(ADMUX,) //Analog Channel Selection (MUX[3:0] = 0000 -> A0)

  //F_ADC = F_CPU / prescaler (usually 128)
  sbi(ADCSRA, ADPS2); sbi(ADCSRA, ADPS1); sbi(ADCSRA, ADPS0);//128

  sbi(ADCSRA,ADEN);//enable ADC

}

uint16_t ADC_10bit() {
  //start conversion (ADSC = 1)
  sbi(ADCSRA,ADSC);

  // wait until end of conversion (ADSC = 0)
  while(bit_is_set(ADCSRA,ADSC));

  // returns result (10-bit from ADC register) (0-1023)
  return ADC;
}


void UART_init(){
    //uint16_t ubrr = 51; //USART Baud Rate Register (holds baud rate value) 51 -> 19200
    UBRR0H = 0; //UBRR is divided between two registers
    UBRR0L = 51;

    sbi(UCSR0C,UCSZ01);sbi(UCSR0C,UCSZ00); //8 bit data

    sbi(UCSR0B,TXEN0);//transmiterr enable 

}

void uart_char(char c){
  while(bit_is_clear(UCSR0A, UDRE0)){}//if buffer is empty
  UDR0 =c; //buffer equals c
  
}

void uart_string(const char* str) {
    while (*str) {
        uart_char(*str++);
    }
}

LiquidCrystal_I2C lcd(0x27, 16, 4);

int main() {
  init();
  
  Wire.begin();
  lcd.init();
  lcd.backlight();
  UART_init();
  ADC_init();

  
  

  lcd.setCursor(0, 0);
  lcd.print("Temperatura: ");


	while(1){
    uint16_t adcValue = ADC_10bit();

    float v = adcValue * (5.0/1024.0)*100;

    lcd.setCursor(13,0);
    lcd.print(v,1);
    
    _delay_ms(500);
  }
}
