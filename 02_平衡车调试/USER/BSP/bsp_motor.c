/*
�ο���https://blog.csdn.net/wangyang666__/article/details/120802967
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
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //ʹ��PB�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	//�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //50M
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);					      //�����趨������ʼ��GPIOB 
}

void bsp_motor_pwm_init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //ʹ��GPIO����ʱ��ʹ��
    
	//���ø�����Ϊ�����������,���TIM1 CH1 CH4��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_11; //TIM_CH1 //TIM_CH4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = arr-1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 
	TIM_TimeBaseStructure.TIM_Prescaler =psc-1; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0;                            //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx

	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE �����ʹ��	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��	 
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH4Ԥװ��ʹ��	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
	TIM_Cmd(TIM1, ENABLE);  //ʹ��TIM1
}

void bsp_encoder_init()
{
    //TIM2
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//ʹ�ܶ�ʱ��4��ʱ��
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ʹ��PB�˿�ʱ��
        {
            GPIO_InitTypeDef GPIO_InitStructure;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	//�˿�����
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
            GPIO_Init(GPIOA, &GPIO_InitStructure);					      //�����趨������ʼ��GPIOB

        }
        {
            TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
            TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
            TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // Ԥ��Ƶ��
            TIM_TimeBaseStructure.TIM_Period = (u16)(65535); //�趨�������Զ���װֵ
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//ѡ��ʱ�ӷ�Ƶ������Ƶ
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;////TIM���ϼ���
            TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

        }
        {
            TIM_ICInitTypeDef TIM_ICInitStructure;
            TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//ʹ�ñ�����ģʽ3
            TIM_ICStructInit(&TIM_ICInitStructure);
            TIM_ICInitStructure.TIM_ICFilter = 10;
            TIM_ICInit(TIM2, &TIM_ICInitStructure);
        }
        TIM_ClearFlag(TIM2, TIM_FLAG_Update);//���TIM�ĸ��±�־λ
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
        //Reset counter
        TIM_SetCounter(TIM2,0);
        TIM_Cmd(TIM2, ENABLE);
    }
	//TIM4
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//ʹ�ܶ�ʱ��4��ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//ʹ��PB�˿�ʱ��
        {
            GPIO_InitTypeDef GPIO_InitStructure;
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//�˿�����
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
			GPIO_Init(GPIOB, &GPIO_InitStructure);					      //�����趨������ʼ��GPIOB
        }
        {
            TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
            TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
            TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // Ԥ��Ƶ��
            TIM_TimeBaseStructure.TIM_Period = (u16)(65535); //�趨�������Զ���װֵ
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//ѡ��ʱ�ӷ�Ƶ������Ƶ
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;////TIM���ϼ���
            TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

        }
        {
            TIM_ICInitTypeDef TIM_ICInitStructure;
            TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//ʹ�ñ�����ģʽ3
			TIM_ICStructInit(&TIM_ICInitStructure);
			TIM_ICInitStructure.TIM_ICFilter = 10;
			TIM_ICInit(TIM4, &TIM_ICInitStructure);
        }
        TIM_ClearFlag(TIM4, TIM_FLAG_Update);//���TIM�ĸ��±�־λ
		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
		//Reset counter
		TIM_SetCounter(TIM4,0);
		TIM_Cmd(TIM4, ENABLE);
    }
}

/**************************************************************************
�������ܣ�TIM4�жϷ�����
��ڲ�������
����  ֵ����
**************************************************************************/
void TIM4_IRQHandler(void)
{
    if(TIM4->SR&0X0001)//����ж�
    {
    }
    TIM4->SR&=~(1<<0);//����жϱ�־λ
}
/**************************************************************************
�������ܣ�TIM2�жϷ�����
��ڲ�������
����  ֵ����
**************************************************************************/
void TIM2_IRQHandler(void)
{
    if(TIM2->SR&0X0001)//����ж�
    {
    }
    TIM2->SR&=~(1<<0);//����жϱ�־λ
}
