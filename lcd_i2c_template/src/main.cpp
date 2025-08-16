#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 4);

int main() {
    
    init();             
    Wire.begin();       

    
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Hello, Kuba!");
    lcd.setCursor(0, 3);
    lcd.print("Arduino LCD");

    
    while (true) {
        
        delay(100);
    }

    return 0;
}
