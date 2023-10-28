#include "bsp_exti.h"

#include "bsp_led.h"

void bsp_exti_init()
{
    //打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    //引脚初始化
    {
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_5;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;

        GPIO_Init(GPIOB,&GPIO_InitStruct);
    }
    //中断线初始化
    {
        EXTI_InitTypeDef EXTI_InitStructure;

        EXTI_InitStructure.EXTI_Line    = EXTI_Line5;
        EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;

        EXTI_Init(&EXTI_InitStructure);

        //初始化中断线
        GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource5);
    }
    //NVIC初始化
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能外部中断通道
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//抢占优先级2，
        NVIC_InitStructure.NVIC_IRQChannelSubPriority =        3;	//子优先级0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//使能外部中断通道

        NVIC_Init(&NVIC_InitStructure);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) == SET)
    {
       
    }
    EXTI_ClearITPendingBit(EXTI_Line5); 
}
