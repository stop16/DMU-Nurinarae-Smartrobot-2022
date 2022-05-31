//로봇제어 라이브러리, 작성자 정지황

#include "Arduino.h"
#include "motor.h"
#include <PRIZM.h>

extern PRIZM prizm;
extern EXPANSION exc;

extern int lbd;
extern int rbd;
extern int lum;
extern int ldm;
extern int rum;
extern int rdm;

void move(char direction, int spd, int time){
	if (direction == 'w'){
		exc.setMotorSpeeds(lbd,spd,spd);
		exc.setMotorSpeeds(rbd,spd,spd);
		delay(time);
	}
	if (direction == 's'){
		exc.setMotorSpeeds(lbd,-1*spd,-1*spd);
		exc.setMotorSpeeds(rbd,-1*spd,-1*spd);
		delay(time);
	}
	if (direction == 'a'){
		exc.setMotorSpeeds(lbd,-1*spd,spd);
		exc.setMotorSpeeds(rbd,-1*spd,spd);
		delay(time);
	}
	if (direction == 'd'){
		exc.setMotorSpeeds(lbd,spd,-1*spd);
		exc.setMotorSpeeds(rbd,spd,-1*spd);
		delay(time);
	}
}

//가속함수
void acc(int boardnum1, int boardnum2, int st_spd, int ed_spd, float time_){
  exc.setMotorSpeeds(boardnum1,st_spd,st_spd);
  exc.setMotorSpeeds(boardnum2,st_spd,st_spd);
  delay(time_/3);
  exc.setMotorSpeeds(boardnum1,(st_spd+ed_spd)/2,(st_spd+ed_spd)/2);
  exc.setMotorSpeeds(boardnum2,(st_spd+ed_spd)/2,(st_spd+ed_spd)/2);
  delay(time_/3);
  exc.setMotorSpeeds(boardnum1,ed_spd,ed_spd);
  exc.setMotorSpeeds(boardnum2,ed_spd,ed_spd);
  delay(time_/3);
}

//정지함수
void stop_robot(){
  exc.setMotorSpeeds(lbd,0,0);
  exc.setMotorSpeeds(rbd,0,0);
  delay(200);
}

void turn(int direction, int spd, int time){
	if (direction == 'a'){
		exc.setMotorSpeeds(lbd,-1*spd,-1*spd);
		exc.setMotorSpeeds(rbd,spd,spd);
		delay(time);
	}
	if (direction == 'd'){
		exc.setMotorSpeeds(lbd,spd,spd);
		exc.setMotorSpeeds(rbd,-1*spd,-1*spd);
		delay(time);
	}
	stop_robot();
}
