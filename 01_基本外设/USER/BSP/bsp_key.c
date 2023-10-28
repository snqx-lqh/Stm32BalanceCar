#include "bsp_key.h"

void bsp_key_init()
{
    //��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    //��ʼ��IO��
    {
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_15;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;

        GPIO_Init(GPIOA,&GPIO_InitStruct);
    }
}

uint8_t get_key_value()
{
	return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15);
}
