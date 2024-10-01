#include "ble_task.h"

#include "control_task.h"

#include "usart.h"
#include "queue.h"

#define  BLE_Receive_Buff_Len     20
#define  BLE_Q_NUM                4
#define  BLE_FRAME_NUM            5  //蓝牙1帧的数据量

uint8_t  BLE_Receive_Buff[BLE_Receive_Buff_Len]={0};
uint8_t  BLE_Solve_Buff[BLE_Receive_Buff_Len]={0};

static QueueHandle_t BLE_Message_Queue;

void ble_task(void const * argument)
{
	uint8_t  Receive_Buff[BLE_Receive_Buff_Len]={0};
	uint8_t  Send_Buff[BLE_Receive_Buff_Len]={0};
	int8_t   speedSet = 0;
	uint8_t  check_sum = 0;
	const balance_car_t *balance_car_temp = get_car_statu();
	
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart3, BLE_Receive_Buff, BLE_Receive_Buff_Len);
	BLE_Message_Queue = xQueueCreate(BLE_Q_NUM,BLE_Receive_Buff_Len);
	
	vTaskDelay(1000);
	while(1)
	{
		if(BLE_Message_Queue!=NULL)
        {
            if(xQueueReceive(BLE_Message_Queue,Receive_Buff,portMAX_DELAY))//请求消息 
            {
				//处理蓝牙接收到的数据
				if(Receive_Buff[0] == 0XA5 && Receive_Buff[4] == 0X5A)
				{
					check_sum = Receive_Buff[1] + Receive_Buff[2];
					speedSet = ((int8_t)Receive_Buff[1])/10;
					if(check_sum == Receive_Buff[3])
					{
						set_car_speed(speedSet);
					}else
					{
						
					}
				}	
				//通过蓝牙发送平衡车数据
				Send_Buff[0] = 0xA5;
				Send_Buff[1] = balance_car_temp->speedNow;
				Send_Buff[2] = balance_car_temp->speedNow >> 8;
				Send_Buff[3] = Send_Buff[1] + Send_Buff[2] ;
				Send_Buff[4] = 0x5A;
				HAL_UART_Transmit(&huart3, Send_Buff, 5,10 );
            }
        } 
	}
}

//串口中断
void USART3_IRQHandler(void)
{
	uint8_t usart3_rx_len = 0;
	BaseType_t xHigherPriorityTaskWoken;
    //当触发了串口空闲中断
    if((__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET)) 
    {
		__HAL_UART_CLEAR_IDLEFLAG(&huart3); //清除空闲标志
		__HAL_DMA_DISABLE(&hdma_usart3_rx); //先停止DMA，暂停接收
		//接收个数等于接收缓冲区总大小减剩余计数
		usart3_rx_len = BLE_Receive_Buff_Len - (__HAL_DMA_GET_COUNTER(&hdma_usart3_rx)); 
		
		if(usart3_rx_len == BLE_FRAME_NUM)
		{
			memcpy(BLE_Solve_Buff,BLE_Receive_Buff,BLE_Receive_Buff_Len);
			xQueueSendFromISR(BLE_Message_Queue,BLE_Solve_Buff,&xHigherPriorityTaskWoken);//向队列中发送数据
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换
			memset(BLE_Receive_Buff,0,BLE_Receive_Buff_Len);
		}		
		hdma_usart3_rx.Instance->CNDTR =  BLE_Receive_Buff_Len;
		__HAL_DMA_ENABLE(&hdma_usart3_rx); 
		
    }
}

void DMA1_Channel2_IRQHandler(void)
{	
	if(__HAL_DMA_GET_FLAG(&hdma_usart3_tx,DMA_FLAG_TC2))
	{
		__HAL_DMA_CLEAR_FLAG(&hdma_usart3_tx,DMA_FLAG_TC2);
		HAL_UART_DMAStop(&huart3);      //传输完成以后关闭串口DMA
	}	
}