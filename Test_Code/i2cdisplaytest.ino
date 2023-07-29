#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x01,16,2);

void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  

}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0,0);
  lcd.print("hell");
}
