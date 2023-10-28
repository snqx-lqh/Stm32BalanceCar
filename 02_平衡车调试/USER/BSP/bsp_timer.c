#include "bsp_timer.h"


void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {
  
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );   //清除TIMx更新中断标志
    }
}

void bsp_timer3_init(uint16_t arr,uint16_t psc)
{
    //时钟初始化
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
    //定时器初始化
    {
        TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

        TIM_TimeBaseStructure.TIM_Period    =   arr-1;                      //设置自动重装载值
        TIM_TimeBaseStructure.TIM_Prescaler =   psc-1;                    //设置预分频值
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;      //设置时钟分割:TDTS = Tck_tim
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式

        TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
    }
    //NVIC初始化
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              //TIM3中断
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;    //先占优先级0级
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;           //从优先级3级
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //IRQ通道被使能

        NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
    }
    //定时器使能以及开启中断
    {
        TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
        TIM_Cmd(TIM3, ENABLE);                    //使能TIMx
    }
}
