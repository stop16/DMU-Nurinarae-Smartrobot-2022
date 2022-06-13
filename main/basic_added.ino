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

#define ff 'w'
#define l 'a'
#define r 's'
#define rr 'd'

char current_robot_direction = ff;
int current_robot_position = 0;
char robot_direction = ff; //로봇 방향
int dir[4] = {2, 2, 2, 2};//값이 0이면 죄측, 1이면 우측, 2이면 모르는 값이다.
int al3;
int al5;


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
  check_value(0);// 오브젝트 체크
  /*(0,x)의 기둥이 노랑이라면, (1,x)로 이동해 잡아온다.(al3에는 (1,x)의 기둥의 색 ID 저장) 그 후, 
   * (2,x)로 이동해 큐브가 al3와 일치하는지 확인 후, 
   * 맞다면 잡아서 (al5에는 (2,x)의 기둥의 색 ID 저장) (1,x)에 놓고, 
   * 아니라면 (3,x)로 이동해 잡아서 (al5에는 (3,x)의 기둥의 색 ID 저장) (1,x)에 놓는다.
   * 
  */
  if(low_cube[0][dir[0]] == 2) 
  {
    mov();
    line();
    check_value(1);
    al3 = low_cube[1][dir[1]];
    mov();
    object_turn_f(1);
    cube_in();
    grip_close();
    cube_out();
    object_turn_f(1);
    line();
    mov();
    object_turn_r(0);
    cube_in();
    grip_open();
    cube_out();
    object_turn_r(0);
    line();
    mov();
    line();
  }
  else if(low_cube[0][dir[0]] != 2)//(0,x)의 기둥이 노랑이 아니라면, 잡고 (1,x)~(3,x)중 노랑에 놓는다.(al3에는 (0,x)의 기둥의 색 ID 저장)
  {
    al3 = low_cube[0][dir[0]];
    mov();
    object_turn_f(0);
    cube_in();
    grip_close();
    cube_out();
    object_turn_r(0);
    line();
    check_value(1);
    if(low_cube[1][dir[1]] == 2)
    {
      mov();
      object_turn_f(1);
      cube_in();
      grip_open();
      cube_out();
      object_turn_r(1);
    }
    else
    {
      mov();
      line();
      check_value(2);
      if(low_cube[2][dir[2]] == 2)
      {
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_r(2);
      }
      else
      {
        mov();
        line();
        check_value(3);
        if(low_cube[3][dir[3]] == 2)
        {
          mov();
          object_turn_f(3);
          cube_in();
          grip_open();
          cube_out();
          object_turn_f(3);
        }
      }
    }
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
    if(digitalRead(2) == 1 && digitalRead(3) == 0 && digitalRead(4) == 0)//왼쪽
    {
      lspd = -10;
      rspd = 160;
    }
    if(digitalRead(2) == 0 && digitalRead(3) == 0 && digitalRead(4) == 1)//오른쪽
    {
      lspd = 160;
      rspd = -10;
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

void object_turn_f(int y)
{
  if(dir[y] == 0)
  {
    left();
  }
  if(dir[y] == 1)
  {
    right();
  }
}

void object_turn_r(int y)
{
  if(dir[y] == 0)
  {
    right();
  }
  if(dir[y] == 1)
  {
    left();
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

void check_value(int y){
  turnhusky_left();
  husky(y,0);
  dir[y] = 0;
  if(low_cube[y][0] == 0){
    turnhusky_right();
    husky(y,1);
    dir[y] = 1;
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
