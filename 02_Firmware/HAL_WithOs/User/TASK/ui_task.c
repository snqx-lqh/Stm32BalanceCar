#include "ui_task.h"

#include "bsp_led.h"
#include "bsp_key.h"

#include "filter.h"
#include "bsp_adc.h"
#include "hc_sr04.h"
 
#include "u8g2.h"
#include "u8g2_stm32_port.h"

void ui_task(void const * argument)
{
	uint8_t disbuff[16]={0};
	uint16_t count = 0;
	float power = 0;
	hc_sr04_init();
	u8g2Init(&u8g2);
	while(1)
	{
		adc_convert();           
		power = get_power_value() ;
		count ++;
		if(count%2 == 0)
			trig_send_pluse();
		
		u8g2_FirstPage(&u8g2);
		do {
			u8g2_SetFontMode(&u8g2, 1);
			u8g2_SetFontDirection(&u8g2, 0);
			u8g2_SetFont(&u8g2, u8g2_font_inb16_mf);
			sprintf((char*)disbuff,"count:%d",count);
			u8g2_DrawStr(&u8g2, 0, 16, (const char*)disbuff);
		} while (u8g2_NextPage(&u8g2));
		 
		vTaskDelay(10);
	}
}


 
 
