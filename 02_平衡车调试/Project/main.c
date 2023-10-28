#include "main.h"

#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_sys.h"

#include "bsp_led.h"
#include "bsp_exti.h"
#include "bsp_timer.h"
#include "bsp_adc.h"
#include "bsp_key.h"
#include "bsp_motor.h"

#include "mpu6050.h"
#include "oled.h"

int16_t leftEncodeTest=0,rightEncodeTest=0;

void example()
{
	//Õ”¬›“«≤‚ ‘
//	MPU_Get_Gyro(&gyrox,&gyroy,&gyroz);
//	MPU_Get_Acc(&accx,&accy,&accz);
//	delay_ms(5);
//	
//	//OLED≤‚ ‘
//	OLED_ShowString(0,0,"hello",16,1);
//	OLED_Refresh();
//	delay_ms(5);
//	
//	//µÁª˙ ‰≥ˆ≤‚ ‘
//	motor_pwm_set(3000,3000);
//	delay_ms(5);
	
	//µÁª˙±‡¬Î∆˜∂¡»°
	
//	leftEncodeTest  += read_encoder(2);
//	rightEncodeTest += read_encoder(4);
//	delay_ms(5);
}


int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	bsp_usart1_init(115200);
	delay_init();
	bsp_led_init();
	bsp_exti_init();
	bsp_timer3_init(5000,72);
	bsp_key_init();
	bsp_motor_init();
	
	MPU_Init();
	OLED_Init();

	while(1)
	{
		//example();
	}
}



