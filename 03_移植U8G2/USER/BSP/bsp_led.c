#include "bsp_led.h"

void bsp_led_init()
{
    //打开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    //初始化IO口
    {
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_13;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStruct.GPIO_Speed= GPIO_Speed_50MHz;

        GPIO_Init(GPIOC,&GPIO_InitStruct);
		
		GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_12;
		
		GPIO_Init(GPIOA,&GPIO_InitStruct);
    }
}

void led_board_on()
{
    GPIO_ResetBits(GPIOC,GPIO_Pin_13);
}

void led_board_off()
{
    GPIO_SetBits(GPIOC,GPIO_Pin_13);
}

void led_red_on()
{
    GPIO_ResetBits(GPIOC,GPIO_Pin_13);
}

void led_red_off()
{
    GPIO_SetBits(GPIOC,GPIO_Pin_13);
}
