#include "ui_task.h"

#include "bsp_led.h"
#include "bsp_key.h"

#include "oled.h"
#include "filter.h"
#include "bsp_adc.h"
#include "hc_sr04.h"
 

void ui_task(void const * argument)
{
	uint8_t showBuff[16]={0};
	uint16_t count = 0;
	float power = 0;
	OLED_Init();
	hc_sr04_init();
	while(1)
	{
		adc_convert();           
		power = get_power_value() ;
		count ++;
		if(count%2 == 0)
			trig_send_pluse();
		OLED_Clear_Buff();
		sprintf((char*)showBuff,"angle %lf",KalmanY.angle);
		OLED_ShowString(0,0,showBuff,16,1);
		sprintf((char*)showBuff,"dis %d",get_hcsr04_mm());
		OLED_ShowString(0,16,showBuff,16,1);
		sprintf((char*)showBuff,"power %.2f",power);
		OLED_ShowString(0,32,showBuff,16,1);
		OLED_Refresh();
		vTaskDelay(10);
	}
}
