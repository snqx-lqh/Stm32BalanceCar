#ifndef _CONTROL_TASK_H
#define _CONTROL_TASK_H

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#define PI 3.14

typedef struct{
	//angle 角度
	int16_t gyro[3];
	int16_t acc[3];
	float   car_angle;
	float   angleAim    ;
	int16_t anglePwm    ;
	int16_t gyroBalance ;
	float   angleKp ; 
	float   angleKd ; 
	
	//speed 速度
	int16_t leftEncode;
	int16_t rightEncode;
	int16_t speedAim ;
	int16_t speedNow ;
	int16_t speedPwm ;
	float   speedKp  ; 
	float   speedKi  ; 
	
	//turn  转向
	int16_t turnAim ;
	int16_t turnNow ;
	int16_t turnPwm ;
	float   turnKp  ; 
	float   turnKd  ;
	
	//pwm输出
	int16_t leftPwm  ;
	int16_t rightPwm ;
	
	//控制模式
	uint8_t control_mode;
}balance_car_t;

void control_task(void const * argument);

const balance_car_t *get_car_statu(void);
void set_car_speed(int16_t speed);

#endif
