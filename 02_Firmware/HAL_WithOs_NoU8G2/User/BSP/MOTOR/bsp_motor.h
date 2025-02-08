#ifndef _BSP_MOTOR_H
#define _BSP_MOTOR_H

#include "main.h"

void motor_init(void);
void motor_set_pwm(int16_t leftPwm,int16_t rightPwm);
int16_t read_encoder(uint8_t TIMX);

#endif
