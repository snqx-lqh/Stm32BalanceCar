#include "bsp_motor.h"
#include "tim.h"

#define MAX_PWM 6900

#define AIN_HIGH()  HAL_GPIO_WritePin(AIN_GPIO_Port,AIN_Pin,GPIO_PIN_SET );
#define AIN_LOW()   HAL_GPIO_WritePin(AIN_GPIO_Port,AIN_Pin,GPIO_PIN_RESET );
#define BIN_HIGH()  HAL_GPIO_WritePin(BIN_GPIO_Port,BIN_Pin,GPIO_PIN_SET );
#define BIN_LOW()   HAL_GPIO_WritePin(BIN_GPIO_Port,BIN_Pin,GPIO_PIN_RESET );

#define SET_PWMA(X) __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4, X)
#define SET_PWMB(X) __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, X)

void motor_init()
{
	HAL_TIM_Base_Start(&htim1);  
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4);
	
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

void motor_set_pwm(int16_t leftPwm,int16_t rightPwm)
{
	if(leftPwm>MAX_PWM)leftPwm = MAX_PWM;
	else if(leftPwm<-MAX_PWM)leftPwm = -MAX_PWM;
	
	if(rightPwm>MAX_PWM)rightPwm = MAX_PWM;
	else if(rightPwm<-MAX_PWM)rightPwm = -MAX_PWM;
	
	if(leftPwm > 0)
	{
		AIN_HIGH();
		SET_PWMA(leftPwm);
	}else{
		AIN_LOW();
		SET_PWMA(-leftPwm);
	}
	
	if(rightPwm > 0)
	{
		BIN_HIGH();
		SET_PWMB(rightPwm);
	}else{
		BIN_LOW();
		SET_PWMB(-rightPwm);
	}
}

int16_t read_encoder(uint8_t TIMX)
{
	int16_t encoder_data = 0;
	switch (TIMX)
	{
		case 2:encoder_data = __HAL_TIM_GET_COUNTER(&htim2);__HAL_TIM_SET_COUNTER(&htim2,0);break;
		case 4:encoder_data = __HAL_TIM_GET_COUNTER(&htim4);__HAL_TIM_SET_COUNTER(&htim4,0);break;
		default:break;
	}
	return encoder_data;
}

