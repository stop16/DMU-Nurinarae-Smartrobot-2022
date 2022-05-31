#include <PRIZM.h>
#include "motor.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "SoftwareSerial.h"
#include "HUSKYLENS.h"

HUSKYLENS huskylens;
SoftwareSerial mySerial(A1,A2);
PRIZM prizm;
EXPANSION exc;
LiquidCrystal_I2C lcd(0x27,16,2);
void printResult(HUSKYLENSResult result);

#define front 'w'
#define left 'a'
#define rear 's'
#define right 'd'


int lbd = 1;
int rbd = 2;
int lum = 1;
int ldm = 2;
int rum = 2;
int rdm = 1;
int h_firstval = 0;
int h_secondval = 0;
int h_thirdval = 0;
int l_firstval = 0;
int l_secondval = 0;
int l_thirdval = 0;
int countval = 0;

float battery_voltage;

void setup() {
  prizm.PrizmBegin();
  Serial.begin(115200);
  mySerial.begin(9600);
  while (!huskylens.begin(mySerial))
    {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
  Wire.begin();
  exc.setMotorInvert(rbd,rdm,1);
  exc.setMotorInvert(rbd,rum,1);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  battery_voltage = prizm.readBatteryVoltage();
  lcd.print(battery_voltage/100);
  lcd.print("V");
  delay(500);
  lcd.setCursor(0,0);
  lcd.print("        ");
  while(1){
  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if(!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
  else{
  Serial.println(F("###########"));
      while (huskylens.available())
      {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);
      displaycolors(result);
      }
    }    
  }
  //move(front, 80, 2000);
  //acc(lbd,rbd,80,10,1000);
  prizm.PrizmEnd();
}


void loop() {
}


void next() {
  while(prizm.readStartButton()==0){
    stop_robot();
  }
}


void runhusky(){
  while(huskylens.available()){
    HUSKYLENSResult result = huskylens.read();
    displaycolors(result);
  }
}

void displaycolors(HUSKYLENSResult result){
  if(result.xCenter <= 100){
    if(result.yCenter <= 50){
      h_firstval = result.ID;
    }
    else{
      l_firstval = result.ID;
    }
  }
  else if(100 < result.xCenter && result.xCenter < 200){
    if(result.yCenter <= 50){
      h_secondval = result.ID;
    }
    else{
      l_secondval = result.ID;
    }
  }
  else if(200 <= result.xCenter){
    if(result.yCenter <= 50){
      h_thirdval = result.ID;
    }
    else{
      l_thirdval = result.ID;
    }
  }
  lcd.setCursor(0,0);
  lcd.print(h_firstval);
  lcd.setCursor(5,0);
  lcd.print(h_secondval);
  lcd.setCursor(10,0);
  lcd.print(h_thirdval);
  lcd.setCursor(0,1);
  lcd.print(l_firstval);
  lcd.setCursor(5,1);
  lcd.print(l_secondval);
  lcd.setCursor(10,1);
  lcd.print(l_thirdval);
  //Serial.println(thirdval);
  countval++;
  if(countval == 30){
    countval = h_firstval= l_firstval = h_secondval = l_secondval = h_thirdval = l_thirdval = 0;
  }
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}
