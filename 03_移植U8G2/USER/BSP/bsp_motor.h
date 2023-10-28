#ifndef _BSP_MOTOR_H
#define _BSP_MOTOR_H

#include "main.h"

#define MAX_PWM_SET 7200

#define L_DIR_1 PBout(12)
#define L_DIR_2 PBout(13)
#define R_DIR_1 PBout(14)
#define R_DIR_2 PBout(15)


int  read_encoder(u8 TIMX);
void bsp_motor_dir_init(void);
void bsp_motor_pwm_init(u16 arr,u16 psc);
void bsp_encoder_init(void);
void bsp_motor_init(void);
void motor_pwm_set(int16_t leftPwm,int16_t rightPwm);



#endif
