#include "hc_sr04.h"
 
#include "tim.h" 
 
#define HCSR04_TIM     htim3
#define HCSR04_TIM_CH  TIM_CHANNEL_3
 
//全局变量
static uint32_t ccr_cnt,period_cnt;//CCR寄存器值，中断次数
static uint8_t  tri_flag,end_flag; //触发方式标志位，捕获标记位
static uint16_t count_cnt_sum  ;   //总的计数值

void hc_sr04_init()
{
	__HAL_TIM_CLEAR_FLAG(&HCSR04_TIM,TIM_IT_UPDATE);//清除中断标志位，防止一使能定时器就进入中断
    HAL_TIM_Base_Start_IT(&HCSR04_TIM);//使能定时器及更新中断
	HAL_TIM_IC_Start_IT(&HCSR04_TIM, HCSR04_TIM_CH);//使能定时器及捕获中断
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
	period_cnt++;//中断计数
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	
	if(tri_flag==0)//上升沿触发时进入(tri_flag  0 上升沿触发 / 1 下降沿触发)
	{
		period_cnt=0;//中断次数清零
		__HAL_TIM_SET_COUNTER(&HCSR04_TIM,0); //计数器清零
		ccr_cnt=0;//存捕获寄存器获取的值的变量清零
		__HAL_TIM_SET_CAPTUREPOLARITY(&HCSR04_TIM, HCSR04_TIM_CH, TIM_INPUTCHANNELPOLARITY_FALLING);//改变触发极性-下降沿触发
		tri_flag=1;
	}
	else//下降沿触发时进入
	{
		ccr_cnt=__HAL_TIM_GET_COMPARE(&HCSR04_TIM, HCSR04_TIM_CH);//获取捕获寄存器的值
		__HAL_TIM_SET_CAPTUREPOLARITY(&HCSR04_TIM, HCSR04_TIM_CH, TIM_INPUTCHANNELPOLARITY_RISING);//改变触发极性-上降沿触发
		tri_flag=0;
		end_flag=1;//捕获完成标志
		if(end_flag==1)//结束捕获
		{
			count_cnt_sum=period_cnt*65535+(ccr_cnt+1); 	 
			end_flag=0;		
		}
 
	}
} 

/**
  * @brief   trig引脚发送信号
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
 
     
 
 