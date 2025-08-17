//LM35
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>


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

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 6 //D8(Arduino) == PB0 (ATmega328P)

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);



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
#define DHT11_PIN 7
#define DHTTYPE DHT11  

int main() {
  init();
  UART_init();
  ADC_init();

  DHT dht(DHT11_PIN, DHTTYPE); 
  dht.begin(); 
  sensors.begin();
  
  Wire.begin(); 

  lcd.init();
  lcd.backlight();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp. analog: ");

  lcd.setCursor(0, 1);
  lcd.print("Temp. DHT: ");

  lcd.setCursor(0, 2);
  lcd.print("Hum. DHT: ");

  lcd.setCursor(0, 3);
  lcd.print("Temp. digital: ");


	while(1){
    uint16_t adcValue = ADC_10bit();
    float temp_analog = adcValue * (5.0/1024.0)*100;

    float temp_dht = dht.readTemperature();
    float hum_dht = dht.readHumidity();

    sensors.requestTemperatures(); // Send the command to get temperatures
    //_delay_ms(750); // 12-bit resolution takes 750ms
    float temp_dig = sensors.getTempCByIndex(0);



    lcd.setCursor(14,0);
    lcd.print(temp_analog,3);
    lcd.setCursor(14,1);
    lcd.print(temp_dht,3);
    lcd.setCursor(14,2);
    lcd.print(hum_dht,3);
    lcd.setCursor(14,3);
    lcd.print(temp_dig,3);

    _delay_ms(200);
    
  }
}
