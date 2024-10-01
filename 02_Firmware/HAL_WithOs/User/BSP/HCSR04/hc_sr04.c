#include "hc_sr04.h"
 
#include "tim.h" 
 
#define HCSR04_TIM     htim3
#define HCSR04_TIM_CH  TIM_CHANNEL_3
 
//ȫ�ֱ���
static uint32_t ccr_cnt,period_cnt;//CCR�Ĵ���ֵ���жϴ���
static uint8_t  tri_flag,end_flag; //������ʽ��־λ��������λ
static uint16_t count_cnt_sum  ;   //�ܵļ���ֵ

void hc_sr04_init()
{
	__HAL_TIM_CLEAR_FLAG(&HCSR04_TIM,TIM_IT_UPDATE);//����жϱ�־λ����ֹһʹ�ܶ�ʱ���ͽ����ж�
    HAL_TIM_Base_Start_IT(&HCSR04_TIM);//ʹ�ܶ�ʱ���������ж�
	HAL_TIM_IC_Start_IT(&HCSR04_TIM, HCSR04_TIM_CH);//ʹ�ܶ�ʱ���������ж�
}
 
uint16_t get_hcsr04_count()
{
	return count_cnt_sum;
}

uint16_t get_hcsr04_mm()
{
	return count_cnt_sum/58.0f;
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	period_cnt++;//�жϼ���
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	
	if(tri_flag==0)//�����ش���ʱ����(tri_flag  0 �����ش��� / 1 �½��ش���)
	{
		period_cnt=0;//�жϴ�������
		__HAL_TIM_SET_COUNTER(&HCSR04_TIM,0); //����������
		ccr_cnt=0;//�沶��Ĵ�����ȡ��ֵ�ı�������
		__HAL_TIM_SET_CAPTUREPOLARITY(&HCSR04_TIM, HCSR04_TIM_CH, TIM_INPUTCHANNELPOLARITY_FALLING);//�ı䴥������-�½��ش���
		tri_flag=1;
	}
	else//�½��ش���ʱ����
	{
		ccr_cnt=__HAL_TIM_GET_COMPARE(&HCSR04_TIM, HCSR04_TIM_CH);//��ȡ����Ĵ�����ֵ
		__HAL_TIM_SET_CAPTUREPOLARITY(&HCSR04_TIM, HCSR04_TIM_CH, TIM_INPUTCHANNELPOLARITY_RISING);//�ı䴥������-�Ͻ��ش���
		tri_flag=0;
		end_flag=1;//������ɱ�־
		if(end_flag==1)//��������
		{
			count_cnt_sum=period_cnt*65535+(ccr_cnt+1); 	 
			end_flag=0;		
		}
 
	}
} 

/**
  * @brief   trig���ŷ����ź�
  * @param    
  * @retval  
 **/ 
void trig_send_pluse(void)
{
	uint32_t i;
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
	for(i=0;i<72*40;i++)
		__NOP();
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
 
}
 
     
 
 