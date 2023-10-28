/*
参考：https://blog.csdn.net/wangyang666__/article/details/120802967
*/

#include "bsp_motor.h"

int read_encoder(u8 TIMX)
{
    int Encoder_TIM;
    switch(TIMX)
    {
    case 2:
        Encoder_TIM= (short)TIM2 -> CNT;
        TIM2 -> CNT=0;
        break;
    case 3:
        Encoder_TIM= (short)TIM3 -> CNT;
        TIM3 -> CNT=0;
        break;
    case 4:
        Encoder_TIM= (short)TIM4 -> CNT;
        TIM4 -> CNT=0;
        break;
    default:
        Encoder_TIM=0;
    }
    return Encoder_TIM;
}

void motor_pwm_set(int16_t leftPwm,int16_t rightPwm)
{
	if(leftPwm > MAX_PWM_SET*0.8) leftPwm = MAX_PWM_SET*0.8;
	else if(leftPwm < -MAX_PWM_SET*0.8) leftPwm = -MAX_PWM_SET*0.8;
	
	if(rightPwm > MAX_PWM_SET*0.8) rightPwm = MAX_PWM_SET*0.8;
	else if(rightPwm < -MAX_PWM_SET*0.8) rightPwm = -MAX_PWM_SET*0.8;
	
	if(leftPwm>0)
	{
		L_DIR_1 = 1;
		L_DIR_2 = 0;
		TIM_SetCompare4(TIM1,leftPwm);
	}else 
	{
		L_DIR_1 = 0;
		L_DIR_2 = 1;
		TIM_SetCompare4(TIM1,-leftPwm);
	}
	
	if(rightPwm>0)
	{
		R_DIR_1 = 0;
		R_DIR_2 = 1;
		TIM_SetCompare1(TIM1,rightPwm);
	}else
	{
		R_DIR_1 = 1;
		R_DIR_2 = 0;
		TIM_SetCompare1(TIM1,-rightPwm);
	}
}

void bsp_motor_init(void)
{
	bsp_motor_dir_init();
	bsp_motor_pwm_init(7200,1); //10K
	bsp_encoder_init();
}

void bsp_motor_dir_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能PB端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //50M
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);					      //根据设定参数初始化GPIOB 
}

void bsp_motor_pwm_init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //使能GPIO外设时钟使能
    
	//设置该引脚为复用输出功能,输出TIM1 CH1 CH4的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_11; //TIM_CH1 //TIM_CH4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = arr-1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 
	TIM_TimeBaseStructure.TIM_Prescaler =psc-1; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;                            //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx

	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE 主输出使能	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1预装载使能	 
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH4预装载使能	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
}

void bsp_encoder_init()
{
    //TIM2
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//使能定时器4的时钟
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能PB端口时钟
        {
            GPIO_InitTypeDef GPIO_InitStructure;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	//端口配置
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
            GPIO_Init(GPIOA, &GPIO_InitStructure);					      //根据设定参数初始化GPIOB

        }
        {
            TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
            TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
            TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // 预分频器
            TIM_TimeBaseStructure.TIM_Period = (u16)(65535); //设定计数器自动重装值
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//选择时钟分频：不分频
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;////TIM向上计数
            TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

        }
        {
            TIM_ICInitTypeDef TIM_ICInitStructure;
            TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//使用编码器模式3
            TIM_ICStructInit(&TIM_ICInitStructure);
            TIM_ICInitStructure.TIM_ICFilter = 10;
            TIM_ICInit(TIM2, &TIM_ICInitStructure);
        }
        TIM_ClearFlag(TIM2, TIM_FLAG_Update);//清除TIM的更新标志位
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
        //Reset counter
        TIM_SetCounter(TIM2,0);
        TIM_Cmd(TIM2, ENABLE);
    }
	//TIM4
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//使能定时器4的时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//使能PB端口时钟
        {
            GPIO_InitTypeDef GPIO_InitStructure;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//端口配置
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
			GPIO_Init(GPIOB, &GPIO_InitStructure);					      //根据设定参数初始化GPIOB
        }
        {
            TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
            TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
            TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // 预分频器
            TIM_TimeBaseStructure.TIM_Period = (u16)(65535); //设定计数器自动重装值
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//选择时钟分频：不分频
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;////TIM向上计数
            TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

        }
        {
            TIM_ICInitTypeDef TIM_ICInitStructure;
            TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//使用编码器模式3
			TIM_ICStructInit(&TIM_ICInitStructure);
			TIM_ICInitStructure.TIM_ICFilter = 10;
			TIM_ICInit(TIM4, &TIM_ICInitStructure);
        }
        TIM_ClearFlag(TIM4, TIM_FLAG_Update);//清除TIM的更新标志位
		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
		//Reset counter
		TIM_SetCounter(TIM4,0);
		TIM_Cmd(TIM4, ENABLE);
    }
}

/**************************************************************************
函数功能：TIM4中断服务函数
入口参数：无
返回  值：无
**************************************************************************/
void TIM4_IRQHandler(void)
{
    if(TIM4->SR&0X0001)//溢出中断
    {
    }
    TIM4->SR&=~(1<<0);//清除中断标志位
}
/**************************************************************************
函数功能：TIM2中断服务函数
入口参数：无
返回  值：无
**************************************************************************/
void TIM2_IRQHandler(void)
{
    if(TIM2->SR&0X0001)//溢出中断
    {
    }
    TIM2->SR&=~(1<<0);//清除中断标志位
}
