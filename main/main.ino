#include <PRIZM.h>
#include "motor.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "SoftwareSerial.h"
#include "HUSKYLENS.h"

HUSKYLENS huskylens;
SoftwareSerial mySerial(A1, A2);
PRIZM prizm;
EXPANSION exc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
int h_secondval = 0;
int l_secondval = 0;
int countval = 0;
float battery_voltage;
int i;
int dir[4] = {2, 2, 2, 2};//값이 0이면 죄측, 1이면 우측, 2이면 모르는 값이다.
int al3;
int al5;

char current_robot_direction = front;
int current_robot_position = 0;
char robot_direction = front; //로봇 방향
int robot_position[5] =
{
  0,
  1,
  2,
  3,
  4
}; //로봇 위치

int cubehigh[4][2] = {
  {-1,-1},
  {-1,-1},
  {-1,-1},
  {-1,-1}
};
int cubelow[4][2] = {
  {-1,-1},
  {-1,-1},
  {-1,-1},
  {-1,-1}
};
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

  exc.setMotorInvert(rbd, rdm, 1);
  exc.setMotorInvert(rbd, rum, 1);
  pinMode(2, INPUT); //IR Front-Left
  pinMode(3, INPUT); //IR Front-Front
  pinMode(4, INPUT); //IR Front-Right
  pinMode(5, INPUT); //IR for grip
  //1이 검정입니다....

  lcd.init();
  lcd.backlight();//lcd 초기설정
  lcd.setCursor(0, 0);
  battery_voltage = prizm.readBatteryVoltage();
  lcd.print(battery_voltage / 100);
  lcd.print("V");
  delay(500);//배터리값 표시

  lcd.setCursor(0, 0);
  lcd.print("        ");//배터리값 지우기

  delay(500);

  prizm.setServoSpeed(1, 60); //서보 속도 설정
  prizm.setServoSpeed(2, 60);

  prizm.setServoPosition(1, 90); //그립 오픈
  prizm.setServoPosition(2, 90);

  //알고리즘 1번 시작

  while (1) {
    move(left, 80, 10);
    if (digitalRead(2) == 1) {
      break;
    }
  }
  move(front, 80, 900);
  while (1) {
    move(front, 80, 10);
    if (digitalRead(2) == 1 && digitalRead(3) == 1 && digitalRead(4) == 1) {
      break;
    }
  }
  move(front, 80, 500);
  while (1) {
    move(left, 80, 10);
    if (digitalRead(2) == 1) {
      break;
    }
  }

  //시작지점에서 중앙 하단부까지 이동함
  //알고리즘 1번 끝
  //알고리즘 2번 시작

  move_to_position(front, 0, front, 1, 80);
  check_value(0, 80);// 왼쪽 돌아 허스키, 오브젝트 없으면 좌로 두번 돌아 허스키, 오브젝트 있는 방향과 로봇이 향하는 방향을 저장함.
  if (cubehigh[0][dir[0]] == 2 || cubelow[0][dir[0]] == 2) // (0,x)칸의 오브젝트에 노랑이 섞여있으면
  {
    cubelow[0][dir[0]] = 2;//기둥 노랑색으로 확정
    move_to_position(get_dir(dir[0]), 1, front, 2, 80); // (1,x)로 이동한다.
    check_value(1, 80); //큐브를 찾아서
    al3 = cubelow[1][dir[1]]; //알고리즘 3번을 위해 기둥 색상값 저장
    ir_grip_seq(80);  //잡고 뒤로 돌아와서
    move_to_position(get_dir(dir[1]), 2, dir_get(dir[0]), 1, 80); //(0,x)로 이동하여
    ir_grip_o_seq(80);  //오브젝트를 놓고 뒤로 돌아온다.
  }
  else // (0,x)칸의 오브젝트에 노랑이 섞여있지 않다면
  {
    al3 = cubelow[0][dir[0]];//알고리즘 3번을 위해 기둥 색상값 저장
    ir_grip_seq(80); // 오브젝트를 잡고 뒤로 돌아온다.
    move_to_position(get_dir(dir[0]), 1, front, 2, 80); // (1,x)로 이동한다.
    check_value(1, 80);
    if (cubehigh[1][dir[1]] == 2 || cubelow[1][dir[1]] == 2) //만약 (1,x)칸의 오브젝트에 노랑이 섞여있다면
    {
      cubelow[1][dir[1]] = 2;//기둥 노랑색으로 확정
      ir_grip_o_seq(80);//오브젝트를 놓고 뒤로 돌아온다.
    }
    else//만약 (1,x)칸의 오브젝트에 노랑이 섞여있지 않다면
    {
      move_to_position(get_dir(dir[1]), 2, front, 3, 80); //(2,x)로 이동한다.
      check_value(2, 80);
      if (cubehigh[2][dir[2]] == 2 || cubelow[2][dir[2]] == 2) //만약 (2,x)칸의 오브젝트에 노랑이 섞여있다면
      {
        cubelow[2][dir[2]] = 2;//기둥 노랑색으로 확정
        ir_grip_o_seq(80);//오브젝트를 놓고 뒤로 돌아온다.
      }
      else//만약 (2,x)칸의 오브젝트에 노랑이 섞여있지 않다면
      {
        move_to_position(get_dir(dir[2]), 3, front, 4, 80); //(3,x)로 이동한다.
        check_value(3, 80);
        //(3,x)칸의 오브젝트에 노랑은 무조건 섞여있음
        cubelow[3][dir[3]] = 2;//기둥 노랑색으로 확정
        ir_grip_o_seq(80);//오브젝트를 놓고 뒤로 돌아온다.
      }
    }
  }
  //알고리즘 2번 끝
  //알고리즘 3,4번 시작

  if (current_robot_position == 1) //현재 위치가 1이면 1은 노랑이므로 3으로 이동해 색 비교 후 al3이 없다면 4로 이동해 색 비교 후 큐브 잡아 2로 돌아와서 다시 놓기
  {
    move_to_position(dir_get(dir[0]), current_robot_position, front, 3, 80); //(2,x)으로 이동
    check_value(3, 80); //3의 값 확인
    if (cubehigh[3][dir[3]] == al3) //3에 al3 큐브가 있다면
    {
      ir_grip_seq(80);//잡아서
      al5 = cubelow[3][dir[3]];
      lcd.setCursor(10,1);
      move_to_position(get_dir(dir[3]), current_robot_position, dir_get(dir[1]), 2, 80); //(1,x)로 돌아와서
      ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
      current_robot_direction = get_dir(dir[2]);
    }
    else//3에 al3 큐브가 없다면
    {
      move_to_position(get_dir(dir[3]), current_robot_position, front, 4, 80); //4로 이동
      check_value(4, 80); //4의 값 확인
      if (cubehigh[4][dir[4]] == al3) //4에 al3 큐브가 있다면
      {
        ir_grip_seq(80);//잡아서
        al5 = cubelow[4][dir[4]];
        move_to_position(get_dir(dir[4]), current_robot_position, dir_get(dir[1]), 2, 80); //2로 돌아와서
        ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
        current_robot_direction = get_dir(dir[2]);
      }
    }
  }
  if (current_robot_position == 2) //현재 위치가 2면 2는 노랑이므로 3으로 이동해 색 비교 후 al3이 없다면 4로 이동해 색 비교 후 큐브 잡아 1로 돌아와서 다시 놓기
  {
    move_to_position(get_dir(dir[1]), current_robot_position, front, 3, 80); //3으로 이동
    check_value(3, 80); //3의 값 확인
    if (cubehigh[3][dir[3]] == al3) //3에 al3 큐브가 있다면
    {
      ir_grip_seq(80);//잡아서
      al5 = cubelow[3][dir[3]];
      move_to_position(get_dir(dir[3]), current_robot_position, dir_get(dir[0]), 1, 80); //1로 돌아와서
      ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
      current_robot_direction = get_dir(dir[1]);
    }
    else//3에 al3큐브가 없다면
    {
      move_to_position(get_dir(dir[1]), current_robot_position, front, 4, 80); //4로 이동
      check_value(4, 80); //4의 값 확인
      if (cubehigh[4][dir[4]] == al3) //4에 al3 큐브가 있다면
      {
        ir_grip_seq(80);//잡아서
        al5 = cubelow[4][dir[4]];
        move_to_position(get_dir(dir[4]), current_robot_position, dir_get(dir[0]), 1, 80); //1로 돌아와서
        ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
        current_robot_direction = get_dir(dir[1]);
      }
    }
  }
  if (current_robot_position == 3) //현재 위치가 3이면 3은 노랑이므로 4로 이동해 색 비교 후 al3이 없다면 2로 이동해 색 비교 후 큐브 잡아 1로 돌아와 다시 놓기
  {
    move_to_position(get_dir(dir[2]), current_robot_position, front, 4, 80); //4로 이동
    check_value(4, 80); //4의 값 확인
    if (cubehigh[4][dir[4]] == al3) //4에 al3 큐브가 있다면
    {
      ir_grip_seq(80);//잡아서
      al5 = cubelow[4][dir[4]];
      move_to_position(get_dir(dir[4]), current_robot_position, dir_get(dir[0]), 1, 80); //1로 돌아와서
      ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
      current_robot_direction = get_dir(dir[1]);
    }
    else//4에 al3 큐브가 없다면
    {
      move_to_position(get_dir(dir[4]), current_robot_position, rear, 2, 80); //2로 이동
      check_value(2, 80); //2의 값 확인
      if (cubehigh[2][dir[2]] == al3) //2에 al3 큐브가 있다면
      {
        ir_grip_seq(80);//잡아서
        al5 = cubelow[2][dir[2]];
        move_to_position(get_dir(dir[2]), current_robot_position, dir_get(dir[0]), 1, 80); //1로 돌아와서
        ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
        current_robot_direction = get_dir(dir[1]);
      }
    }
  }
  if (current_robot_position == 4) //현재 위치가 4면 4는 노랑이므로 3으로 이동해 색 비교 후 al3이 없다면 2로 이동해 색 비교후 큐브 잡아 1로 돌아와 다시 놓기
  {
    move_to_position(get_dir(dir[3]), current_robot_position, rear, 3, 80); //3으로 이동
    if (cubehigh[3][dir[3]] == al3) //3에 aL3 큐브가 있다면
    {
      move_to_position(get_dir(dir[3]), current_robot_position, dir_get(dir[3]), 3, 80);
      ir_grip_seq(80);//잡아서
      al5 = cubelow[3][dir[3]];
      move_to_position(get_dir(dir[3]), current_robot_position, dir_get(dir[0]), 1, 80); //1로 돌아와서
      ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
      current_robot_direction = get_dir(dir[1]);
    }
    else//3에 al3큐브가 없다면
    {
      move_to_position(get_dir(dir[3]), current_robot_position, dir_get(dir[1]), 2, 80); //2로 이동
      ir_grip_seq(80);//잡아서
      al5 = cubelow[2][dir[2]];
      move_to_position(get_dir(dir[2]), current_robot_position, dir_get(dir[0]), 1, 80); //1로 돌아와서
      ir_grip_o_seq(80);//놓고 뒤로 돌아온다.
      current_robot_direction = get_dir(dir[1]);
    }
  }
  //알고리즘 3,4번 끝
  //알고리즘 5,6번 시작
  int al5_c_p = current_robot_position;
  if(cubehigh[0][0] == al5 || cubehigh[0][1] == al5){
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[0]),0,80);
    ir_grip_seq(80);
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[al5_c_p-1]),al5_c_p,80);
    ir_grip_o_seq(80);
  }
  if(cubehigh[1][0] == al5 || cubehigh[1][1] == al5){
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[1]),1,80);
    ir_grip_seq(80);
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[al5_c_p-1]),al5_c_p,80);
    ir_grip_o_seq(80);
  }
  if(cubehigh[2][0] == al5 || cubehigh[2][1] == al5){
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[2]),2,80);
    ir_grip_seq(80);
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[al5_c_p-1]),al5_c_p,80);
    ir_grip_o_seq(80);
  }
  if(cubehigh[3][0] == al5 || cubehigh[3][1] == al5){
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[3]),3,80);
    ir_grip_seq(80);
    move_to_position(current_robot_direction,current_robot_position,dir_get(dir[al5_c_p-1]),al5_c_p,80);
    ir_grip_o_seq(80);
  }
  else{
    
  }

  //알고리즘 5,6번 끝
  //알고리즘 7,8번 시작

  int al7_y;
  int al7_n = 10 - (2 + al3 + al5);
  int k;
  for (al7_y = 5; al7_y < 4; al7_y++) //노랑 y좌표 검색
  {
    if (cubelow[al7_y][0] == 2 || cubelow[al7_y][1] == 2)
    {
      break;
    }
  }
  if (al7_y < 5) //노랑이 검색됐다면
  {
    move_to_position(current_robot_direction, current_robot_position, dir_get(al7_y), al7_y, 80); //노랑으로 이동
    ir_grip_seq(80);//잡고
    for (k = 0; k < 5; k++) //노랑,al3,al5가 아닌 큐브 위치 검색
    {
      if (cubelow[k][0] == al7_n || cubelow[k][1] == al7_n)
      {
        break;
      }
    }
    move_to_position(current_robot_direction, current_robot_position, dir_get(k), k, 80); //남는 한자리로 이동
    ir_grip_o_seq(80);
  }

  //알고리즘 7,8번 끝
  //알고리즘 9번 시작
  move_to_position(current_robot_direction, current_robot_position, rear, 1, 80); //1로 이동
  line_move(80);//0에 걸치게 이동
  current_robot_position = 0;
  lcd.setCursor(10, 0);
  lcd.print(current_robot_position);
  move(rear, 80, 1000); //뒤로 살짝 이동
  while (1) //IR2에 닿을때까지 우로 이동
  {
    move(right, 80, 10);
    if (digitalRead(2) == 1)
    {
      break;
    }
  }
  while (1) //IR2,3,4 닿을때까지 전진
  {
    move(front, 80, 10);
    if (digitalRead(2) == digitalRead(3) == digitalRead(4) == 1)
    {
      break;
    }
  }
  move(front, 80, 300);
  while (1) //IR2,3,4 닿을때까지 전진
  {
    move(front, 80, 10);
    if (digitalRead(2) == digitalRead(3) == digitalRead(4) == 1)
    {
      break;
    }
  }
  move(rear, 80, 300);
  move(right, 80, 500);
  //알고리즘 9번 끝

  prizm.PrizmEnd();
}


void loop() {
}

void check_value(int y, int spd) {
  turn_sens(left, 80);
  husky_30(y, 0);
  if (cubehigh[y][0] > 0 || cubelow[y][0] > 0) {
    dir[y] = 0;
    current_robot_direction = left;
  }
  else {
    dir[y] = 1;
    current_robot_direction = right;
    turn_sens(left, 80);
    turn_sens(left, 80);
    husky_30(y, 1);
  }
}

char dir_get(int val) {
  if (val == 0) {
    return left;
  }
  if (val == 1) {
    return right;
  }
}

char get_dir(int val) {
  if (val == 0) {
    return left;
  }
  if (val == 1) {
    return right;
  }
}

int t_location(int val) {
  for (int a = 0; a < 4; a++) {
    for (int b = 0; b < 4; b++) {
      if (cubehigh[a][b] == val) {
        return a;
      }
    }
  }
}

char t_direction(int val) {
  for (int a = 0; a < 4; a++) {
    for (int b = 0; b < 4; b++) {
      if (cubehigh[a][b] == val) {
        if (b == 1) {
          return right;
        }
        if (b == 0) {
          return left;
        }
      }
    }
  }
}

//초록버튼 누를때까지 동작 멈추는 함수
void next() {
  while (prizm.readStartButton() == 0) {
    stop_robot();
  }
}


void ir_grip(int spd) {
  while (1) {
    move(front, spd - 50, 10);
    if (digitalRead(5) == 0) {
      break;
    }
  }
  stop_robot();
  grip_close();
  delay(1000);
}

void ir_grip_seq(int spd) {
  while (1) {
    line_trace(spd - 50, 5);
    if (digitalRead(5) == 0) {
      break;
    }
  }
  stop_robot();
  grip_close();
  delay(1300);
  if (current_robot_direction = left) move(rear, spd, 860);
  else if (current_robot_direction = right) move(rear, spd, 940);
}

void ir_grip_o(int spd) {
  while (1) {
    line_trace(spd - 50, 5);
    if (digitalRead(5) == 0) {
      break;
    }
  }
  stop_robot();
  grip_open();
  delay(1000);
}

void ir_grip_o_seq(int spd) {
  while (1) {
    line_trace(spd - 50, 5);
    if (digitalRead(5) == 0) {
      break;
    }
  }
  stop_robot();
  grip_open();
  delay(1300);
  if (current_robot_direction = left) move(rear, spd, 860);
  else if (current_robot_direction = right) move(rear, spd, 940);
}

void grip_close() {
  prizm.setServoPosition(1, 180);
  prizm.setServoPosition(2, 10);
}

void grip_open() {
  prizm.setServoPosition(1, 90);
  prizm.setServoPosition(2, 90);
}

void line_move(int spd) { //한칸 앞으로 가는 함수

  while (1) {
    move(front, spd, 10);
    if (digitalRead(2) == 1 && digitalRead(3) == 0 && digitalRead(4) == 0) {
      while (1) {
        exc.setMotorSpeeds(lbd, -spd, -spd);
        exc.setMotorSpeeds(rbd, spd, spd);
        delay(20);
        if (digitalRead(3) == 1) {
          break;
        }
      }
    }
    if (digitalRead(4) == 1 && digitalRead(3) ==  0 && digitalRead(2) == 0) {
      while (1) {
        exc.setMotorSpeeds(lbd, spd, spd);
        exc.setMotorSpeeds(rbd, -spd, -spd);
        delay(20);
        if (digitalRead(3) == 1) {
          break;
        }
      }
    }
    if (digitalRead(2) == 1 && digitalRead(4) == 1) {
      break;
    }
  }

}

void line_trace(int spd, int times) {
  for (int i = 0; i < times; i++) {
    move(front, spd, 10);
    if (digitalRead(2) == 1 && digitalRead(3) == digitalRead(4) == 0) {
      while (1) {
        exc.setMotorSpeeds(lbd, -spd, -spd);
        exc.setMotorSpeeds(rbd, spd, spd);
        delay(20);
        if (digitalRead(3) == 1) {
          break;
        }
      }
    }
    if (digitalRead(4) == 1 && digitalRead(3) == digitalRead(2) == 0) {
      while (1) {
        exc.setMotorSpeeds(lbd, spd, spd);
        exc.setMotorSpeeds(rbd, -spd, -spd);
        delay(20);
        if (digitalRead(3) == 1) {
          break;
        }
      }
    }
  }
}

void turn_sens(char dir, int spd) {
  if (dir == 'a') {
    turn(left, spd, 700);
    while (1) {
      exc.setMotorSpeeds(lbd, -spd, -spd);
      exc.setMotorSpeeds(rbd, spd, spd);
      delay(15);
      if (digitalRead(3) == 1 && digitalRead(2) == 0) {
        break;
      }

    }
    stop_robot();
  }
  if (dir == 'd') {
    turn(right, spd, 700);
    while (1) {
      exc.setMotorSpeeds(lbd, spd, spd);
      exc.setMotorSpeeds(rbd, -spd, -spd);
      delay(15);
      if (digitalRead(3) == 1 && digitalRead(4) == 0) {
        break;
      }

    }
    stop_robot();
  }
}

void set_dir(char s_dir, char t_dir, int spd) {
  if (s_dir == 'w') {
    if (t_dir == 'a') {
      turn_sens(left, spd);
    }
    if (t_dir == 'd') {
      turn_sens(right, spd);
    }
    if (t_dir == 's') {
      turn_sens(left, spd);
      turn_sens(left, spd);
    }
    else {}
  }
  if (s_dir == 'a') {
    if (t_dir == 'w') {
      turn_sens(right, spd);
    }
    if (t_dir == 's') {
      turn_sens(left, spd);
    }
    if (t_dir == 'd') {
      turn_sens(left, spd);
      turn_sens(left, spd);
    }
    else {}
  }
  if (s_dir == 's') {
    if (t_dir == 'w') {
      turn_sens(left, spd);
      turn_sens(left, spd);
    }
    if (t_dir == 'a') {
      turn_sens(right, spd);
    }
    if (t_dir == 'd') {
      turn_sens(left, spd);
    }
    else {}
  }
  if (s_dir == 'd') {
    if (t_dir == 'w') {
      turn_sens(left, spd);
    }
    if (t_dir == 'a') {
      turn_sens(left, spd);
      turn_sens(left, spd);
    }
    if (t_dir == 's') {
      turn_sens(right, spd);
    }
    else {}
  }
}

void move_to_position(char s_dir, char s_pos, char t_dir, char t_pos, int spd)
{
  if (s_pos < t_pos) {
    set_dir(s_dir, front, spd);
    for (int j = s_pos; j < t_pos; j++) {
      line_move(spd);
      move(front, 80, 550);
    }
    stop_robot();
    set_dir(front, t_dir, spd);
    current_robot_position = t_pos;
    lcd.setCursor(10, 0);
    lcd.print(current_robot_position);
    current_robot_direction = t_dir;
  }
  if (s_pos == t_pos) {
    set_dir(s_dir, t_dir, spd);
    current_robot_position = t_pos;
    current_robot_direction = t_dir;
    lcd.setCursor(10, 0);
    lcd.print(current_robot_position);
  }
  if (s_pos > t_pos) {
    set_dir(s_dir, rear, spd);
    for (int j = s_pos; j > t_pos; j--) {
      line_move(spd);
      stop_robot();
      delay(500);
      move(front, 80, 650);
      stop_robot();
      delay(500);
    }
    stop_robot();
    current_robot_position = t_pos;
    lcd.setCursor(10, 0);
    lcd.print(current_robot_position);
    set_dir(rear, t_dir, spd);
    current_robot_direction = t_dir;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void husky_30(int y, int x) {
  lcd.setCursor(0, 0);
  lcd.print("  ");
  lcd.setCursor(0, 1);
  lcd.print("  ");
  huskylens.request();
  HUSKYLENSResult result_a;
  HUSKYLENSResult result_b;
  result_a = huskylens.getBlock(0);
  result_b = huskylens.getBlock(1);
  if (result_b.yCenter < result_a.yCenter) {
    h_secondval = result_b.ID;
    l_secondval = result_a.ID;
  }
  else {
    h_secondval = result_a.ID;
    l_secondval = result_b.ID;
  }
  lcd.setCursor(0, 0);
  lcd.print(h_secondval);
  lcd.setCursor(0, 1);
  lcd.print(l_secondval);
  cubehigh[y][x] = h_secondval;
  cubelow[y][x] = l_secondval;
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  }
  else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  }
  else {
    Serial.println("Object unknown!");
  }
}
