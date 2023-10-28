#include "bsp_exti.h"

#include "bsp_led.h"

void bsp_exti_init()
{
    //��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    //���ų�ʼ��
    {
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_5;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;

        GPIO_Init(GPIOB,&GPIO_InitStruct);
    }
    //�ж��߳�ʼ��
    {
        EXTI_InitTypeDef EXTI_InitStructure;

        EXTI_InitStructure.EXTI_Line    = EXTI_Line5;
        EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;

        EXTI_Init(&EXTI_InitStructure);

        //��ʼ���ж���
        GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource5);
    }
    //NVIC��ʼ��
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ���ⲿ�ж�ͨ��
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//��ռ���ȼ�2��
        NVIC_InitStructure.NVIC_IRQChannelSubPriority =        3;	//�����ȼ�0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ���ⲿ�ж�ͨ��

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
