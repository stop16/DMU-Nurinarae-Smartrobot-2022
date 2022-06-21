#include <PRIZM.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <HUSKYLENS.h>

HUSKYLENS huskylens;
SoftwareSerial mySerial(A1, A2);
PRIZM prizm;
EXPANSION exc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int dir[4] = {2, 2, 2, 2};//값이 0이면 죄측, 1이면 우측, 2이면 모르는 값이다.
int al3;
int al5;
int knowledge = 0;
int al5_y;
int al7_cf;
int al7;
int bf;

char* c[3] = {"BLUE","GREEN","RED"};
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
    delay(10);
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
  delay(100);//배터리값 표시
  prizm.setServoSpeed(1, 40); //서보 속도 설정
  prizm.setServoSpeed(2, 40);
  prizm.setServoPosition(1, 90); //그립 오픈
  prizm.setServoPosition(2, 90);
  //코드부분/////////////////////////////////////////
  turnhusky_front();
  //cube_in();
  //grip_close();
  //wheel(0,0,260);//X = 좌우 이동, Y = 좌우 회전, Z = 전후 이동
  /*
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1,0,0,80,720);
  exc.setMotorTargets(2,0,0,80,720);
  while (exc.readMotorBusy(1,2)) {}
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1,0,0,360,720);
  exc.setMotorTargets(2,0,0,360,720);
  while (exc.readMotorBusy(1,2)) {}
  */
  //prizm.PrizmEnd();
  opening();
  line();
  location(1);
  check_value(0);// 오브젝트 체크
  /*(0,x)의 기둥이 노랑이라면, (1,x)로 이동해 잡아온다.(al3에는 (1,x)의 기둥의 색 ID 저장) 그 후,
     (2,x)로 이동해 큐브가 al3와 일치하는지 확인 후,
     맞다면 잡아서 (al5에는 (2,x)의 기둥의 색 ID 저장) (1,x)에 놓고, 앞을 보도록 돌려놓는다.
     아니라면 (3,x)로 이동해 잡아서 (al5에는 (3,x)의 기둥의 색 ID 저장) (1,x)에 놓고, 앞을 보도록 돌려놓는다.
  */
  if (low_cube[0][dir[0]] == 2)
  {
    mov();
    line();location(2);
    check_value(1);
    al3 = low_cube[1][dir[1]];
    mov();
    object_turn_f(1);
    cube_in();
    grip_close();
    cube_out();
    object_turn_f(1);
    line();location(1);
    mov();
    object_turn_r(0);
    cube_in();
    grip_open();
    cube_out();
    object_turn_r(0);
    line();location(2);
    movv();
    line();location(3);
    check_value(2);
    if (high_cube[2][dir[2]] == al3) {
      al5 = low_cube[2][dir[2]];
      al5_y = 2;
      mov();
      object_turn_f(2);
      cube_in();
      grip_close();
      cube_out();
      object_turn_f(2);
      line();location(2);
      movv();
      object_turn_r(1);
      cube_in();
      grip_open();
      cube_out();
      object_turn_r(1);
    }
    else if (high_cube[2][dir[2]] != al3) {
      mov();
      line();location(4);
      check_value(3);
      al5 = low_cube[3][dir[3]];
      al5_y = 3;
      mov();
      object_turn_f(3);
      cube_in();
      grip_close();
      cube_out();
      object_turn_f(3);
      line();location(3);
      movv();
      line();location(2);
      mov();
      object_turn_r(1);
      cube_in();
      grip_open();
      cube_out();
      object_turn_r(1);
    }
  }
  /* (0,x)의 기둥이 노랑이 아니라면, 잡고 (1,x)~(3,x)중 노랑에 놓는다.(al3에는 (0,x)의 기둥의 색 ID 저장)
     (1,x)에 있다면, (2,x)로 이동해 al3와 비교한 후, 같다면 잡아서 (0,x)에 놓고, 아니라면 (3,x)로 이동해 잡고 (0,x)에 놓은 뒤, (1,x)로 이동한다.
     (2,x)에 있다면, (1,x)와 al3을 비교한 후, 같다면 잡아서 (0,x)에 놓고, 아니라면 (3,x)로 이동해 잡고 (0,x)에 놓은 뒤, (1,x)로 이동한다.
     (3,x)에 있다면, (1,x)와 (2,x)중 큐브가 있는 곳으로 이동해 잡아서 (0,x)에 놓은 뒤, (1,x)로 이동한다.
  */
  else if (low_cube[0][dir[0]] != 2)
  {
    al3 = low_cube[0][dir[0]];
    mov();
    object_turn_f(0);
    cube_in();
    grip_close();
    cube_out();
    object_turn_r(0);
    line();location(2);
    check_value(1);
    if (low_cube[1][dir[1]] == 2)
    {
      mov();
      object_turn_f(1);
      cube_in();
      grip_open();
      cube_out();
      object_turn_r(1);//(1,x)에서 앞을 보는 상태
      line();location(3);
      check_value(2);
      if (high_cube[2][dir[2]] == al3)
      {
        mov();
        object_turn_f(2);
        cube_in();
        grip_close();
        al5 = low_cube[2][dir[2]];
        al5_y = 2;
        cube_out();
        object_turn_f(2);
        line();location(2);
        movv();
        line();location(1);
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_r(0);
        line();location(2);
        mov();
      }
      else if (high_cube[2][dir[2]] != al3)
      {
        movv();
        line();location(4);
        check_value(3);
        mov();
        object_turn_f(3);
        cube_in();
        grip_close();
        al5 = low_cube[3][dir[3]];
        al5_y = 3;
        cube_out();
        object_turn_f(3);
        line();location(3);
        movv();
        line();location(2);
        movv();
        line();location(1);
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_r(0);
        line();location(2);
        mov();
      }
    }
    else
    {
      movv();
      line();location(3);
      check_value(2);
      if (low_cube[2][dir[2]] == 2)
      {
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        if (high_cube[1][dir[1]] == al3)
        {
          object_turn_f(2);
          line();location(1);
          mov();
          al5 = low_cube[1][dir[1]];
          al5_y = 1;
          cube_in();
          grip_close();
          cube_out();
          object_turn_r(1);
          line();location();
          mov();
          object_turn_r(0);
          cube_in();
          grip_open();
          cube_out();
          object_turn_r(0);
          line();location();
          mov();
        }
        else if (high_cube[1][dir[1]] != al3)
        {
          object_turn_r(2);
          line();location();
          check_value(3);
          mov();
          al5 = low_cube[3][dir[3]];
          al5_y = 3;
          object_turn_f(3);
          cube_in();
          grip_close();
          cube_out();
          object_turn_f(3);
          line();location();
          movv();
          line();location();
          movv();
          line();location();
          mov();
          object_turn_r(0);
          cube_in();
          grip_open();
          cube_out();
          object_turn_r(0);
          line();location();
          mov();
        }
      }
      else
      {
        mov();
        line();location();
        check_value(3);
        if (low_cube[3][dir[3]] == 2)
        {
          mov();
          object_turn_f(3);
          cube_in();
          grip_open();
          cube_out();
          object_turn_f(3);//(3,x)에서 뒤를 보는 상태
          if (high_cube[1][dir[1]] == al3)
          {
            line();location();
            movv();
            line();location();
            mov();
            al5 = low_cube[1][dir[1]];
            al5_y = 1;
            object_turn_r(1);
            cube_in();
            grip_close();
            cube_out();
            object_turn_f(1);
            line();location();
            mov();
            object_turn_r(0);
            cube_in();
            grip_open();
            cube_out();
            object_turn_r(0);
            line();location();
            mov();
          }
          else if (high_cube[2][dir[2]] == al3)
          {
            line();location();
            mov();
            al5 = low_cube[2][dir[2]];
            al5_y = 2;
            object_turn_r(2);
            cube_in();
            grip_close();
            cube_out();
            object_turn_f(2);
            line();location();
            movv();
            line();location();
            mov();
            object_turn_r(0);
            cube_in();
            grip_open();
            cube_out();
            object_turn_r(0);
            line();location();
            mov();
          }
        }
      }
    }
  }

  if (knowledge == 4) //모두 스캔했다면 al5 찾는다.
  {
    if (high_cube[1][dir[1]] == al5) {
      object_turn_f(1);
      cube_in();
      grip_close();
      cube_out();
      object_turn_r(1);
      if (al5_y == 2) {
        line();location();
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);//(2,x)에서 뒤를 보고있다.
        al7_cf = 2;
      }
      else if (al5_y == 3) {
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);//(3,x)에서 뒤를 보고있다.
        al7_cf = 3;
      }
    }
    if (high_cube[2][dir[2]] == al5) //1에 al5기둥이 있는 경우, 3에 al5기둥이 있는 경우
    {
      line();location();
      mov();
      object_turn_f(2);
      cube_in();
      grip_close();
      cube_out();
      if (al5_y == 1) {
        object_turn_f(2);
        line();location();
        mov();
        object_turn_r(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_r(1);//(1,x)에서 앞을 보고있다.
        al7_cf = 1;
      }
      else if (al5_y == 3) {
        object_turn_r(2);
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);//(3,x)에서 뒤를 보고있다.
        al7_cf = 3;
      }
    }
    if (high_cube[3][dir[3]] == al5) //1에 al5기둥이 있는 경우, 2에 al5기둥이 있는 경우
    {
      line();location();
      movv();
      line();location();
      mov();
      object_turn_f(3);
      cube_in();
      grip_close();
      cube_out();
      object_turn_f(3);
      line();location();
      mov();
      if (al5_y == 2) {
        object_turn_r(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);//(2,x)에서 뒤를 보고있다.
        al7_cf = 2;
      }
      else if (al5_y == 1) {
        line();location();
        mov();
        object_turn_r(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_r(1);//(1,x)에서 앞을 보고있다.
        al7_cf = 1;
      }
    }

  }
  if (knowledge == 3) //(3,x)로 가서 잡은 뒤, al5_y가 2,1,0일 때 할 행동 지정
  {
    line();location();
    movv();
    line();location();
    check_value(3);
    mov();
    object_turn_f(3);
    cube_in();
    grip_close();
    cube_out();
    object_turn_f(3);
    if (al5_y == 2) {
      line();location();
      mov();
      object_turn_r(2);
      cube_in();
      grip_open();
      cube_out();
      object_turn_f(2);//(2,x)에서 뒤를 보고있다.
      al7_cf = 2;
    }
    else if (al5_y == 1) {
      line();location();
      movv();
      line();location();
      mov();
      object_turn_r(1);
      cube_in();
      grip_open();
      cube_out();
      object_turn_r(1);//(1,x)에서 앞을 보고있다.
      al7_cf = 1;
    }
    else if (al5_y == 0) {
      line();location();
      movv();
      line();location();
      movv();
      line();location();
      mov();
      object_turn_r(0);
      cube_in();
      grip_open();
      cube_out();
      object_turn_r(0);
      line();location();
      mov();//(1,x)에서 앞을 보고있다.
      al7_cf = 1;
    }
  }

  //노랑으로 가서 잡고 (10 - 2 - al3 - al5) 색의 기둥으로 가서 놓고 (0,x)에서 뒤를 보도록 한다.
  al7 = 10 - 2 - al3 - al5;
  if (al7_cf == 1) {
    if (low_cube[0][dir[0]] == 2) {
      right();
      right();
      line();location();
      mov();
      object_turn_r(0);
      cube_in();
      grip_close();
      cube_out();
      object_turn_r(0);
      if (low_cube[1][dir[1]] == al7) {
        line();location();
        mov();
        object_turn_f(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(1);
        line();location();
        mov();
      }
      if (low_cube[2][dir[2]] == al7) {
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
      }
      if (low_cube[3][dir[3]] == al7) {
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
      }
    }
    if (low_cube[2][dir[2]] == 2) {
      line();location();
      mov();
      object_turn_f(2);
      cube_in();
      grip_close();
      cube_out();
      if (low_cube[0][dir[0]] == al7) {
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(0);
      }
      if (low_cube[1][dir[1]] == al7) {
        object_turn_f(2);
        line();location();
        mov();
        object_turn_r(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(1);
        line();location();
        mov();
      }
      if (low_cube[3][dir[3]] == al7) {
        object_turn_r(2);
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
      }
    }
    if (low_cube[3][dir[3]] == 2) {
      line();location();
      movv();
      line();location();
      mov();
      object_turn_f(3);
      cube_in();
      grip_close();
      cube_out();
      object_turn_f(3);
      if (low_cube[0][dir[0]] == al7) {
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(0);
      }
      if (low_cube[1][dir[1]] == al7) {
        line();location();
        movv();
        line();location();
        mov();
        object_turn_r(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(1);
        line();location();
        mov();
      }
      if (low_cube[2][dir[2]] == al7) {
        line();location();
        mov();
        object_turn_r(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
      }
    }
  }
  if (al7_cf == 2) {
    if (low_cube[0][dir[0]] == 2) {
      line();location();
      movv();
      line();location();
      mov();
      object_turn_r(0);
      cube_in();
      grip_close();
      cube_out();
      object_turn_r(0);
      if (low_cube[1][dir[1]] == al7) {
        line();location();
        mov();
        object_turn_f(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(1);
        line();location();
        mov();
      }
      if (low_cube[2][dir[2]] == al7) {
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
      }
      if (low_cube[3][dir[3]] == al7) {
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
      }
    }
    if (low_cube[1][dir[1]] == 2) {
      line();location();
      mov();
      object_turn_r(1);
      cube_in();
      grip_close();
      cube_out();
      if (low_cube[0][dir[0]] == al7) {
        object_turn_f(1);
        line();location();
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(0);
      }
      if (low_cube[2][dir[2]] == al7) {
        object_turn_r(1);
        line();location();
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
      }
      if (low_cube[3][dir[3]] == al7) {
        object_turn_r(1);
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
      }
    }
    if (low_cube[3][dir[3]] == 2) {
      right();
      right();
      line();location();
      mov();
      object_turn_f(3);
      cube_in();
      grip_close();
      cube_out();
      object_turn_f(3);
      if (low_cube[0][dir[0]] == al7) {
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(0);
      }
      if (low_cube[1][dir[1]] == al7) {
        line();location();
        movv();
        line();location();
        mov();
        object_turn_r(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(1);
        line();location();
        mov();
      }
      if (low_cube[2][dir[2]] == al7) {
        line();location();
        mov();
        object_turn_r(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
      }
    }
  }
  if (al7_cf == 3) {
    if (low_cube[0][dir[0]] == 2) {
      line();location();
      movv();
      line();location();
      movv();
      line();location();
      mov();
      object_turn_r(0);
      cube_in();
      grip_close();
      cube_out();
      object_turn_r(0);
      if (low_cube[1][dir[1]] == al7) {
        line();location();
        mov();
        object_turn_f(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(1);
        line();location();
        mov();
      }
      if (low_cube[2][dir[2]] == al7) {
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
      }
      if (low_cube[3][dir[3]] == al7) {
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
      }
    }
    if (low_cube[1][dir[1]] == 2) {
      line();location();
      movv();
      line();location();
      mov();
      object_turn_r(1);
      cube_in();
      grip_close();
      cube_out();
      if (low_cube[0][dir[0]] == al7) {
        object_turn_f(1);
        line();location();
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(0);
      }
      if (low_cube[2][dir[2]] == al7) {
        object_turn_r(1);
        line();location();
        mov();
        object_turn_f(2);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
      }
      if (low_cube[3][dir[3]] == al7) {
        object_turn_r(1);
        line();location();
        movv();
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
      }
    }
    if (low_cube[2][dir[2]] == 2) {
      line();location();
      mov();
      object_turn_r(2);
      cube_in();
      grip_close();
      cube_out();
      if (low_cube[0][dir[0]] == al7) {
        object_turn_f(2);
        line();location();
        movv();
        line();location();
        mov();
        object_turn_r(0);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(0);
      }
      if (low_cube[1][dir[1]] == al7) {
        object_turn_f(2);
        line();location();
        mov();
        object_turn_r(1);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(1);
        line();location();
        mov();
      }
      if (low_cube[3][dir[3]] == al7) {
        object_turn_r(2);
        line();location();
        mov();
        object_turn_f(3);
        cube_in();
        grip_open();
        cube_out();
        object_turn_f(3);
        line();location();
        movv();
        line();location();
        movv();
        line();location();
        mov();
      }
    }
  }
  ending();
  prizm.PrizmEnd();
}

void loop() {
}


//구동계열

void wheel(int x, int y, int z) // Omni wheel 이동공식
{
  int A = 0, B = 0, C = 0, D = 0;

  A = (x * 0.5) + (y * 0.5) + (z * 0.841471);
  B = (x * 0.5 * -1) + (y * 0.5) + (z * 0.841471);
  C = (x * 0.5 * -1) + (y * 0.5 * -1) + (z * 0.841471);
  D = (x * 0.5) + (y * 0.5 * -1) + (z * 0.841471);

  exc.setMotorSpeeds(1, A, B);
  exc.setMotorSpeeds(2, C, D);
}

void mov()
{
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, 140, 180, 140, 180);
  exc.setMotorTargets(2, 160, 180, 160, 180);
  while (exc.readMotorBusy(1, 1)) {}
}

void movv()
{
  wheel(0,0,240);
  delay(100);
}


void opening()
{
  lcd.setCursor(8,0);
  lcd.print("START");
  while (1) {
    exc.setMotorSpeeds(1, -120, 120);
    exc.setMotorSpeeds(2, -120, 120);
    if (digitalRead(4) == 1)
    {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
  }
  while (1) {
    exc.setMotorSpeeds(1, 120, 120);
    exc.setMotorSpeeds(2, 120, 120);
    if (digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1)
    {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
  }
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, 80, 180, 80, 180);
  exc.setMotorTargets(2, 80, 180, 80, 180);
  while (exc.readMotorBusy(1, 1)) {}
  while (1)
  {
    exc.setMotorSpeeds(1, -80, 80);
    exc.setMotorSpeeds(2, -80, 80);
    if (digitalRead(3) == 1)
    {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
  }
}

void ending()
{
  line_s(); // 마동지막 칸 까지 이
  //마지막 세개 인식 이후 동작임
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, -160, -180, -160, -180);
  exc.setMotorTargets(2, -160, -180, -160, -180);
  while (exc.readMotorBusy(1, 1)) {}
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, 160, 720, -160, -720);
  exc.setMotorTargets(2, 160, 720, -160, -720);
  while (exc.readMotorBusy(1, 1)) {}
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, 160, 1440, 160, 1440);
  exc.setMotorTargets(2, 160, 1440, 160, 1440);
  while (exc.readMotorBusy(1, 1)) {}
  lcd.setCursor(8,0);
  lcd.print("      ");
  lcd.setCursor(8,0);
  lcd.print("FINISH");
}

void line()
{

  while(1)
  {
    if(digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1)
    {
      wheel(0,0,0);
      break;
    }
    if(digitalRead(2) == 1 && digitalRead(3) == 0 && digitalRead(4) == 0)
    {
      wheel(0,-80,150);
    }
    if(digitalRead(2) == 0 && digitalRead(3) == 0 && digitalRead(4) == 1)
    {
      wheel(0,80,150);
    }
    else
    {
      wheel(-50,0,250);
    }
    
  }
  /*
  int lspd, rspd;
  lspd = rspd = 120;
  while (1) {
    exc.setMotorSpeeds(1, lspd, lspd);
    exc.setMotorSpeeds(2, rspd, rspd);
    if (digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1)
    {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
    if (digitalRead(2) == 1 && digitalRead(3) == 0 && digitalRead(4) == 0) //왼쪽
    {
      lspd = -30;
      rspd = 90;
    }
    if (digitalRead(2) == 0 && digitalRead(3) == 0 && digitalRead(4) == 1) //오른쪽
    {
      lspd = 90;
      rspd = -30;
    }
    if (digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 0) //중앙
    {
      lspd = 120;
      rspd = 120;
    }
  }
  */
}

void line_s()
{
  int lspd, rspd;
  lspd = rspd = 120;
  while (1) {
    exc.setMotorSpeeds(1, lspd, lspd);
    exc.setMotorSpeeds(2, rspd, rspd);
    if (digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1)
    {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
    if (digitalRead(2) == 1 && digitalRead(3) == 0 && digitalRead(4) == 0) //왼쪽
    {
      lspd = -80;
      rspd = 80;
    }
    if (digitalRead(2) == 0 && digitalRead(3) == 0 && digitalRead(4) == 1) //오른쪽
    {
      lspd = 80;
      rspd = -80;
    }
    if (digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 0) //중앙
    {
      lspd = 80;
      rspd = 80;
    }
  }
}

void left()
{
  exc.setMotorSpeeds(1, -160, -160);
  exc.setMotorSpeeds(2, 160, 160);
  delay(500);
  while (1) {
    exc.setMotorSpeeds(1, -80, -80);
    exc.setMotorSpeeds(2, 80, 80);
    delay(5);
    if (digitalRead(2) == 0 && digitalRead(3) == 1) {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
  }
}

void right()
{
  exc.setMotorSpeeds(1, 160, 160);
  exc.setMotorSpeeds(2, -160, -160);
  delay(500);
  while (1) {
    exc.setMotorSpeeds(1, 80, 80);
    exc.setMotorSpeeds(2, -80, -80);
    delay(5);
    if (digitalRead(4) == 0 && digitalRead(3) == 1) {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
  }
}

void object_turn_f(int y)
{
  if (dir[y] == 0)
  {
    left();
  }
  if (dir[y] == 1)
  {
    right();
  }
}

void object_turn_r(int y)
{
  if (dir[y] == 0)
  {
    right();
  }
  if (dir[y] == 1)
  {
    left();
  }
}



//허스키렌즈 계열

void husky_grip()
{
  HUSKYLENSResult result;
  huskylens.request();
  while(huskylens.count() > 0)
  {
    huskylens.request();
    result = huskylens.getBlock(0);
    if(result.width>=10){
      if(result.xCenter<=120)
      {
        exc.setMotorSpeeds(1,-30,30);
        exc.setMotorSpeeds(2,-30,30);
      }
      else if(result.xCenter<=220)
      {
        exc.setMotorSpeeds(1,30,30);
        exc.setMotorSpeeds(2,30,30);
      }
      else if(result.xCenter<=285)
      {
        exc.setMotorSpeeds(1,30,-30);
        exc.setMotorSpeeds(2,30,-30);
      }
    }
  }
  exc.setMotorSpeeds(1,0,0);
  exc.setMotorSpeeds(2,0,0);
}

void turnhusky_left()
{
  delay(400);
  prizm.setServoPosition(3, 170);
  delay(400);
}

void turnhusky_right()
{
  delay(400);
  prizm.setServoPosition(3, 10);
  delay(400);
}

void turnhusky_front()
{
  delay(400);
  prizm.setServoPosition(3, 80);
  delay(400);
}

char* color(int x)
{
  if(x == 1){
    return c[0];
  }
  if(x == 3){
    return c[1];
  }
  if(x == 4){
    return c[2];
  }
}

void husky(int y, int x)
{
  huskylens.request();
  int num = huskylens.count();
  if (num == 0)
  {
    high_cube[y][x] = 0;
    low_cube[y][x] = 0;
    lcd.setCursor(8, 0);
    lcd.print("      ");
    lcd.setCursor(8, 0);
    lcd.print("VOID");
    lcd.setCursor(8, 1);
    lcd.print("      ");
    lcd.setCursor(8, 1);
    lcd.print("VOID");
  }
  if (num == 1)
  {
    high_cube[y][x] = 0;
    low_cube[y][x] = 2;
    lcd.setCursor(8, 0);
    lcd.print("      ");
    lcd.setCursor(8, 0);
    lcd.print("VOID");
    lcd.setCursor(8, 1);
    lcd.print("      ");
    lcd.setCursor(8, 1);
    lcd.print("YELLOW");
  }
  if (num == 2)
  {
    HUSKYLENSResult result_1 = huskylens.get(0);
    HUSKYLENSResult result_2 = huskylens.get(1);
    if (result_1.ID == 2 || result_2.ID == 2)
    {
      high_cube[y][x] = 0;
      low_cube[y][x] = 2;
      lcd.setCursor(8, 0);
      lcd.print("      ");
      lcd.setCursor(8, 0);
      lcd.print("VOID");
      lcd.setCursor(8, 1);
      lcd.print("      ");
      lcd.setCursor(8, 1);
      lcd.print("YELLOW");
    }
    else if (result_1.yCenter > result_2.yCenter)
    {
      high_cube[y][x] = result_2.ID;
      low_cube[y][x] = result_1.ID;
      lcd.setCursor(8, 0);
      lcd.print("      ");
      lcd.setCursor(8, 0);
      lcd.print(color(result_2.ID));
      lcd.setCursor(8, 1);
      lcd.print("      ");
      lcd.setCursor(8, 1);
      lcd.print(color(result_1.ID));
    }
    else if (result_1.yCenter < result_2.yCenter)
    {
      high_cube[y][x] = result_1.ID;
      low_cube[y][x] = result_2.ID;
      lcd.setCursor(8, 0);
      lcd.print("      ");
      lcd.setCursor(8, 0);
      lcd.print(color(result_1.ID));
      lcd.setCursor(8, 1);
      lcd.print("      ");
      lcd.setCursor(8, 1);
      lcd.print(color(result_2.ID));
    }
  }
}

void check_value(int y) {
  turnhusky_left();
  husky(y, 0);
  dir[y] = 0;
  if (low_cube[y][0] == 0) {
    turnhusky_right();
    husky(y, 1);
    dir[y] = 1;
  }
  knowledge++;
  turnhusky_front();
}

//그립계열

void grip_open()
{
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, -40, -30, -40, -30);
  exc.setMotorTargets(2, -40, -30, -40, -30);
  bf = 1;
  while (exc.readMotorBusy(1, 1)) {}
  delay(200);
  prizm.setServoPosition(1, 90);
  prizm.setServoPosition(2, 90);
  delay(1000);
}

void grip_close()
{
  delay(200);
  prizm.setServoPosition(1, 180);
  prizm.setServoPosition(2, 10);
  delay(1000);
}

void cube_in()
{
  while(1)
  {
    if(digitalRead(5) == 0)
    {
      wheel(0,0,0);
      break;
    }
    if(digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 0)
    {
      while(1)
      {
        wheel(0,-25,0);
        if(digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 0)
        {
          break;
        }
      }
    }
    if(digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 1)
    {
      while(1)
      {
        wheel(0,25,0);
        if(digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 0)
        {
          break;
        }
      }
    }
    else
    {
      wheel(0,0,40);
    }
    
  }
  
  /*
  int lspd, rspd;
  lspd = rspd = 30;
  while (1) {
    exc.setMotorSpeeds(1, lspd, lspd);
    exc.setMotorSpeeds(2, rspd, rspd);
    if (digitalRead(5) == 0)
    {
      exc.setMotorSpeeds(1, 0, 0);
      exc.setMotorSpeeds(2, 0, 0);
      break;
    }
    if (digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 0) //왼쪽
    {
      lspd = -30;
      rspd = 30;
    }
    if (digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 1) //오른쪽
    {
      lspd = 30;
      rspd = -30;
    }
    if (digitalRead(2) == 0 && digitalRead(3) == 1 && digitalRead(4) == 0) //중앙
    {
      lspd = 30;
      rspd = 30;
    }
  }
  */
  //husky_grip();
}

void cube_out()
{
  if(bf == 1){
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, -120, -240, -120, -240);
  exc.setMotorTargets(2, -120, -240, -120, -240);
  while (exc.readMotorBusy(1, 1)) {}
  }
  else{
  exc.resetEncoders(1);
  exc.resetEncoders(2);
  exc.setMotorTargets(1, -120, -270, -120, -270);
  exc.setMotorTargets(2, -120, -270, -120, -270);
  while (exc.readMotorBusy(1, 1)) {}}
}

//LCD계열

void location(int y)
{
  lcd.setCursor(16,0);
  lcd.print(y);
}
