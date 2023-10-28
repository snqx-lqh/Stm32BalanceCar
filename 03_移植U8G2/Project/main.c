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
#include "stm32_u8g2.h"

int16_t leftEncodeTest=0,rightEncodeTest=0;


u8g2_t u8g2; 

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
	u8g2Init(&u8g2);

//	draw(&u8g2);

	while(1)
	{
		draw(&u8g2);
	}
}



