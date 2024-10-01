#include "bsp_led.h"

void led_on(void)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET );
}

void led_off(void)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET );
}

void led_toggle(void)
{
	HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
}
