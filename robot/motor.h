//구동부 헤더파일, 작성자 정지황

#ifndef motor_H
#define motor_H

void move(char direction, int spd, int time);
void acc(int boardnum1, int boardnum2, int st_spd, int ed_spd, float time_);
void stop_robot();
void turn(int direction, int spd, int time);

#endif