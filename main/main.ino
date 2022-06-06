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

#define front 'w'
#define left 'a'
#define rear 's'
#define right 'd'


//기본 변수들
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

int cubehigh[4][2];
int cubelow[4][2];
//큐브위치,[세로][가로]

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
  pinMode(2,INPUT);//IR Front-Left
  pinMode(3,INPUT);//IR Front-Front
  pinMode(4,INPUT);//IR Front-Right
  //1이 검정입니다....
  
  lcd.init();
  lcd.backlight();//lcd 초기설정
  
  lcd.setCursor(0,0);
  battery_voltage = prizm.readBatteryVoltage();
  lcd.print(battery_voltage/100);
  lcd.print("V");
  delay(500);//배터리값 표시

  lcd.setCursor(0,0);
  lcd.print("        ");//배터리값 지우기

  prizm.setServoSpeed(1,30);//서보 속도 설정
  prizm.setServoSpeed(2,30);

  prizm.setServoPosition(1,90);//그립 오픈
  prizm.setServoPosition(2,90);


  while(1){
    move(left,80,10);
    if(digitalRead(2)==1){
      break;
    }
  }
  move(front,80,1000);
  while(1){
  move(front,80,10);
  if(digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1){
    break;
  }
  }
  move(front,80,300);
  while(1){
    move(left,80,10);
    if(digitalRead(3)==1){
      break;
    }
  }

//시작지점에서 중앙 하단부까지 이동함

  line_move(80);

  move(front,80,600);
  turn(left,80,200);
  while(1){
    exc.setMotorSpeeds(lbd,-80,-80);
    exc.setMotorSpeeds(rbd,80,80);
    delay(15);
    if(digitalRead(2)==1){
      if(digitalRead(3)==1){
        break;
      }
    }
  }

  stop_robot();

  delay(500);

  line_trace(80,40);

  for(int i = 0; i<30; i++){
    runhusky(0,0);
  }

  delay(1000);
  grip_close();
  delay(1000);

  line_move(-80);

  move(front,80,500);

  turn(right,80,200);
  while(1){
    exc.setMotorSpeeds(lbd,80,80);
    exc.setMotorSpeeds(rbd,-80,-80);
    delay(15);
    if(digitalRead(4) == 1){
      if(digitalRead(3) == 1){
        break;
      }
    }
  }

  stop_robot();

  /*

  move(right,120,400);
  while(1){
    exc.setMotorSpeeds(lbd,80,-80);
    exc.setMotorSpeeds(rbd,80,-80);
    delay(15);
    if(digitalRead(3)==1){
        break;
    }
  }

  stop_robot();

  delay(500);

  move(rear,80,100);

  move(right,120,400);
  while(1){
    exc.setMotorSpeeds(lbd,80,-80);
    exc.setMotorSpeeds(rbd,80,-80);
    delay(15);
    if(digitalRead(3)==1){
        break;
    }
  }

  stop_robot();

  delay(500);

  move(right,120,400);
  while(1){
    exc.setMotorSpeeds(lbd,80,-80);
    exc.setMotorSpeeds(rbd,80,-80);
    delay(15);
    if(digitalRead(3)==1){
        break;
    }
  }

  stop_robot();

  delay(500);

  */
  
  /*
  for(int i = 0; i<30; i++){
    runhusky(0);
  }
  //lcd.setCursor(0,0);
  Serial.print(leftlow_1);
  //lcd.setCursor(2,0);
  Serial.print(lefthigh_1);
  */
  
  prizm.PrizmEnd();
}


void loop() {
}

//초록버튼 누를때까지 동작 멈추는 함수
void next() {
  while(prizm.readStartButton()==0){
    stop_robot();
  }
}

//허스키렌즈 1회 동작, for/while문과 조합해서 쓰세용
void runhusky(int column, int row){
      if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
      else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
      else if(!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
      else{
          while (huskylens.available())
          {
          HUSKYLENSResult result = huskylens.read();
          displaycolors(result);
          cubelow[column][row] = l_secondval;
          cubehigh[column][row] = h_secondval;
          
          }
      }
  }

void displaycolors(HUSKYLENSResult result){
  if(result.xCenter <= 100){
    if(result.yCenter <= 60){
      h_firstval = result.ID;
    }
    else{
      l_firstval = result.ID;
    }
  }
  else if(100 < result.xCenter && result.xCenter < 200){
    if(result.yCenter <= 60){
      h_secondval = result.ID;
    }
    else{
      l_secondval = result.ID;
    }
  }
  else if(200 <= result.xCenter){
    if(result.yCenter <= 60){
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
  countval++;
  if(countval == 30){
    countval = h_firstval= l_firstval = h_secondval = l_secondval = h_thirdval = l_thirdval = 0;
  }
}

void grip_close(){
  prizm.setServoPosition(1,180);
  prizm.setServoPosition(2,10);
}

void grip_open(){
  prizm.setServoPosition(1,90);
  prizm.setServoPosition(2,90);
}

void line_move(int spd){//한칸 앞으로 가는 함수
  
  while(1){
    move(front,spd,10);
    if(digitalRead(2) == 1 && digitalRead(3) == digitalRead(4) == 0){
      while(1){
        exc.setMotorSpeeds(lbd,-spd,-spd);
        exc.setMotorSpeeds(rbd,spd,spd);
        delay(15);
        if(digitalRead(3) == 1){
          break;
        }
      }
    }
    if(digitalRead(4) == 1 && digitalRead(3) == digitalRead(2) == 0){
      while(1){
        exc.setMotorSpeeds(lbd,spd,spd);
        exc.setMotorSpeeds(rbd,-spd,-spd);
        delay(15);
        if(digitalRead(3) == 1){
          break;
        }
      }
    }
    if(digitalRead(2)==1&&digitalRead(4)==1){
      break;
    }
  }

}

void line_trace(int spd, int times){
    for(int i = 0; i<times; i++){
    move(front,spd,10);
    if(digitalRead(2) == 1 && digitalRead(3) == digitalRead(4) == 0){
      while(1){
        exc.setMotorSpeeds(lbd,-spd,-spd);
        exc.setMotorSpeeds(rbd,spd,spd);
        delay(15);
        if(digitalRead(3) == 1){
          break;
        }
      }
    }
    if(digitalRead(4) == 1 && digitalRead(3) == digitalRead(2) == 0){
      while(1){
        exc.setMotorSpeeds(lbd,spd,spd);
        exc.setMotorSpeeds(rbd,-spd,-spd);
        delay(15);
        if(digitalRead(3) == 1){
          break;
        }
      }
    }
  }
  stop_robot();

}
