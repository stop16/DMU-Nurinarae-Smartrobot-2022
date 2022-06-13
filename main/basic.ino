#include <PRIZM.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <HUSKYLENS.h>

HUSKYLENS huskylens;
SoftwareSerial mySerial(A1,A2);
PRIZM prizm;
EXPANSION exc;
LiquidCrystal_I2C lcd(0x27,16,2);

float battery_voltage;
int high_cube[4][2] = {
  {0, 0},
  {0, 0},
  {0, 0},
  {0, 0}
};

int low_cube[4][2] = {
  {0, 0},
  {0, 0},
  {0, 0},
  {0, 0}
};

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
  exc.setMotorInvert(2, 1, 1);
  exc.setMotorInvert(2, 2, 1);
  pinMode(2, INPUT); //IR Front-Left
  pinMode(3, INPUT); //IR Front-Front
  pinMode(4, INPUT); //IR Front-Right
  pinMode(5, INPUT); //IR for grip
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  battery_voltage = prizm.readBatteryVoltage();
  lcd.print(battery_voltage / 100);
  lcd.print("V");
  delay(500);//배터리값 표시
  delay(500);
  prizm.setServoSpeed(1, 60); //서보 속도 설정
  prizm.setServoSpeed(2, 60);
  prizm.setServoPosition(1, 90); //그립 오픈
  prizm.setServoPosition(2, 90);
//코드부분/////////////////////////////////////////

  opening();
  line();
  turnhusky_left();
  husky(0,0);
  if(low_cube[0][0] == 0)
  {
    turnhusky_right();
    husky(0,1);
  }
  mov();
  //(0,x) 좌,우측 감지 완료
  if(low_cube[0][0] == 2 || low_cube[0][1] == 2)//(0,x) 좌,우측에 노랑이 있다면
  {
    //두번째로 가서 잡고 첫번째에 놓는다.
    line();
    turnhusky_left();
    husky(1,0);
    if(low_cube[1][0] == 0)
    {
      turnhusky_right();
      husky(1,1);
    }
    mov();
    if(low_cube[1][0] != 0)
    {
      left();
      cube_in();
      delay(1000);
      cube_out();
      left();
    }
    else if(low_cube[1][1] != 0)
    {
      right();
      cube_in();
      delay(1000);
      cube_out();
      right();
    }
    line();
    mov();
    if(low_cube[0][0] == 2){
      right();
      cube_in();
      delay(1000);
      cube_out();
      right();
    }
    else if(low_cube[0][1] == 2)
    {
      left();
      cube_in();
      delay(1000);
      cube_out();
      left();
    }
    line();mov();line();//세 번째 칸으로 이동한다.
  }

  
  
  prizm.PrizmEnd();
}

void loop() {
}


//구동계열
void mov()
{
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1,80,180,80,180);
  exc.setMotorTargets(2,80,180,80,180);
  while(exc.readMotorBusy(1,1)){}
}

void opening()
{
  while(1){
    exc.setMotorSpeeds(1,-80,80);
    exc.setMotorSpeeds(2,-80,80);
    if(digitalRead(4) == 1)
    {
      exc.setMotorSpeeds(1,0,0);
      exc.setMotorSpeeds(2,0,0);
      break;
    }
  }
  while(1){
    exc.setMotorSpeeds(1,80,80);
    exc.setMotorSpeeds(2,80,80);
    if(digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1)
    {
      exc.setMotorSpeeds(1,0,0);
      exc.setMotorSpeeds(2,0,0);
      break;
    }
  }
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1,80,180,80,180);
  exc.setMotorTargets(2,80,180,80,180);
  while(exc.readMotorBusy(1,1)){}
  while(1)
  {
    exc.setMotorSpeeds(1,-80,80);
    exc.setMotorSpeeds(2,-80,80);
    if(digitalRead(3) == 1)
    {
      exc.setMotorSpeeds(1,0,0);
      exc.setMotorSpeeds(2,0,0);
      break;
    }
  }
  
}

void line()
{
  int lspd, rspd;
  lspd = rspd = 160;
  while(1){
    exc.setMotorSpeeds(1,lspd,lspd);
    exc.setMotorSpeeds(2,rspd,rspd);
    delay(5);
    if(digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1)
    {
      exc.setMotorSpeeds(1,0,0);
      exc.setMotorSpeeds(2,0,0);
      break;
    }
    if(digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 0)//왼쪽
    {
      lspd = 50;
      rspd = 160;
    }
    if(digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 1)//오른쪽
    {
      lspd = 160;
      rspd = 50;
    }
    if(digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 0)//중앙
    {
      lspd = 160;
      rspd = 160;
    }
    }
}

void left()
{
  exc.setMotorSpeeds(1,-160,-160);
  exc.setMotorSpeeds(2,160,160);
  delay(500);
  while(1){
    exc.setMotorSpeeds(1,-80,-80);
    exc.setMotorSpeeds(2,80,80);
    delay(5);
    if(digitalRead(2) == 0 && digitalRead(3) == 1){
      exc.setMotorSpeeds(1,0,0);
      exc.setMotorSpeeds(2,0,0);
      break;
    }
  }
}

void right()
{
  exc.setMotorSpeeds(1,160,160);
  exc.setMotorSpeeds(2,-160,-160);
  delay(500);
  while(1){
    exc.setMotorSpeeds(1,80,80);
    exc.setMotorSpeeds(2,-80,-80);
    delay(5);
    if(digitalRead(4) == 0 && digitalRead(3) == 1){
      exc.setMotorSpeeds(1,0,0);
      exc.setMotorSpeeds(2,0,0);
      break;
    }
  }
}

//허스키렌즈 계열

void turnhusky_left()
{
  delay(400);
  prizm.setServoPosition(3,180);
  delay(400);
}

void turnhusky_right()
{
  delay(400);
  prizm.setServoPosition(3,10);
  delay(400);
}

void husky(int y, int x)
{
  huskylens.request();
  int num = huskylens.count();
  if(num == 0)
  {
    high_cube[y][x] = 0;
    low_cube[y][x] = 0;
    lcd.setCursor(8,0);
    lcd.print("0");
    lcd.setCursor(8,1);
    lcd.print("0");
  }
  if(num == 1)
  {
    high_cube[y][x] = 0;
    low_cube[y][x] = 2;
    lcd.setCursor(8,0);
    lcd.print("0");
    lcd.setCursor(8,1);
    lcd.print("2");
  }
  if(num == 2)
  {
    HUSKYLENSResult result_1 = huskylens.get(0);
    HUSKYLENSResult result_2 = huskylens.get(1);
    if(result_1.ID == 2 || result_2.ID == 2)
    {
      high_cube[y][x] = 0;
      low_cube[y][x] = 2;
      lcd.setCursor(8,0);
      lcd.print("0");
      lcd.setCursor(8,1);
      lcd.print("2");
    }
    else if(result_1.yCenter > result_2.yCenter)
    {
      high_cube[y][x] = result_2.ID;
      low_cube[y][x] = result_1.ID;
      lcd.setCursor(8,0);
      lcd.print(result_2.ID);
      lcd.setCursor(8,1);
      lcd.print(result_1.ID);
    }
    else if(result_1.yCenter < result_2.yCenter)
    {
      high_cube[y][x] = result_1.ID;
      low_cube[y][x] = result_2.ID;
      lcd.setCursor(8,0);
      lcd.print(result_1.ID);
      lcd.setCursor(8,1);
      lcd.print(result_2.ID);
    }
  }
}

//그립계열

void grip_open()
{
  delay(1000);
  prizm.setServoPosition(1,90);
  prizm.setServoPosition(2,90);
  delay(1000);
}

void grip_close()
{
  delay(1000);
  prizm.setServoPosition(1,180);
  prizm.setServoPosition(2,10);
  delay(1000);
}

void cube_in()
{
  int lspd, rspd;
  lspd = rspd = 30;
  while(1){
    exc.setMotorSpeeds(1,lspd,lspd);
    exc.setMotorSpeeds(2,rspd,rspd);
    if(digitalRead(5) == 0)
    {
      exc.setMotorSpeeds(1,0,0);
      exc.setMotorSpeeds(2,0,0);
      break;
    }
    if(digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 0)//왼쪽
    {
      lspd = -30;
      rspd = 30;
    }
    if(digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 1)//오른쪽
    {
      lspd = 30;
      rspd = -30;
    }
    if(digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 0)//중앙
    {
      lspd = 30;
      rspd = 30;
    }
    }
}

void cube_out()
{
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1,-40,-270,-40,-270);
  exc.setMotorTargets(2,-40,-270,-40,-270);
  while(exc.readMotorBusy(1,1)){}
}
