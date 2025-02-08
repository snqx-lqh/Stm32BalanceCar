/**
  ******************************************************************************
  * @file    ble_task.c
  * @author  少年潜行(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   这一部分是蓝牙调试器的接受任务，我主要使用这一部分来进行蓝牙上位机
			 调试小车的PID
			 这部分的主要方案是通过串口空闲中断加DMA接受蓝牙数据，接受到的数据放入
			 定义好的蓝牙串口信息队列，然后在任务中检测到队列中有数据，就处理相关的
			 数据。
			 蓝牙协议帧的定义
			 接受帧：  接收帧主要调试PID，传过来的值还需要根据实际情况处理，比如取反
			 ------------------------------------------------------------------------------------------
			 |帧头|角度KP低8位|角度KP高8位|角度KD低8位|角度KD高8位|速度KP低8位|速度KP高8位|校验和|帧尾|
			 |  0 |      1    |      2    |      3    |      4    |      5    |      6    |   7  |  8 |
			 |0xA5|      -    |      -    |      -    |      -    |      -    |      -    |   -  |0X5A|
			 ------------------------------------------------------------------------------------------
			 发送帧：  发送帧主要查看角度变化
			 ------------------------------------------------------------------------------------------
			 |帧头|角度低8位|角度高8位|校验和|帧尾|
			 |  0 |    1    |    2    |  3   | 4  |
			 |0xA5|    -    |    -    |  -   |0X5A|
			 ------------------------------------------------------------------------------------------
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; Copyright {Year} LQH,China</center></h2>
  ******************************************************************************
  */
#include "ble_task.h"

#include "control_task.h"

#include "usart.h"
#include "queue.h"

#define  BLE_Receive_Buff_Len     20 //设置接受数据的最大缓冲值
#define  BLE_Q_NUM                4  //设置队列的长度
#define  BLE_FRAME_NUM            9  //蓝牙1帧的数据量

uint8_t  BLE_Receive_Buff[BLE_Receive_Buff_Len]={0};  //蓝牙数据接受数组
uint8_t  BLE_Solve_Buff[BLE_Receive_Buff_Len]  ={0};    //将接受到的数据复制的数组，好像没有这一步也没啥问题

static QueueHandle_t BLE_Message_Queue; //蓝牙串口数据缓冲队列

/**
  * @brief   蓝牙任务处理 
  * @param    
  * @retval
 **/
void ble_task(void const * argument)
{
	uint8_t  Receive_Buff[BLE_Receive_Buff_Len]={0}; //队列中取出数据的临时存放缓冲
	uint8_t  Send_Buff[BLE_Receive_Buff_Len]={0};    //发送数据的缓冲数组
	
	int16_t  angle = 0;      //小车角度值
	float    angleKpSet = 0,angleKdSet = 0,speedKpSet = 0;   //小车PID设置值
	uint8_t  check_sum = 0;  //校验和值
	const balance_car_t *balance_car_temp = get_car_statu(); //获得平衡车的变量地址
	
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);    //打开串口3空闲中断
	HAL_UART_Receive_DMA(&huart3, BLE_Receive_Buff, BLE_Receive_Buff_Len); //设置DMA的BUFF和长度
	BLE_Message_Queue = xQueueCreate(BLE_Q_NUM,BLE_Receive_Buff_Len); //创建蓝牙数据队列
	
	vTaskDelay(1000);
	while(1)
	{
		if(BLE_Message_Queue!=NULL)
        {
            if(xQueueReceive(BLE_Message_Queue,Receive_Buff,portMAX_DELAY))//如果蓝牙队列有数据 
            {
				//处理蓝牙接收到的数据
				if(Receive_Buff[0] == 0XA5 && Receive_Buff[8] == 0X5A)
				{
					//校验和检查
					check_sum = Receive_Buff[1] + Receive_Buff[2] + Receive_Buff[3] + Receive_Buff[4] + Receive_Buff[5] + Receive_Buff[6];
					if(check_sum == Receive_Buff[7])
					{
						//解析数据，并且KD传过来的数据是按比例放大了的，需要除回去
						angleKpSet = (float)((Receive_Buff[2] << 8) + Receive_Buff[1]);
						angleKdSet = (float)((Receive_Buff[4] << 8) + Receive_Buff[3]);
						speedKpSet = (float)((Receive_Buff[6] << 8) + Receive_Buff[5]);
						angleKpSet = -angleKpSet;
						angleKdSet = -angleKdSet/100;
						speedKpSet = -speedKpSet;
						//设置小车PID参数
						set_car_pid(angleKpSet,angleKdSet,speedKpSet);
					}else
					{
						
					}
				}	
				//通过蓝牙发送平衡车角度数据
				angle = (int)(balance_car_temp->car_angle * 10);
				Send_Buff[0] = 0xA5;
				Send_Buff[1] = angle;
				Send_Buff[2] = angle >> 8;
				Send_Buff[3] = Send_Buff[1] + Send_Buff[2] ;
				Send_Buff[4] = 0x5A;
				HAL_UART_Transmit(&huart3, Send_Buff, 5,10 );
            }
        } 
	}
}

/**
  * @brief   蓝牙串口中断处理 
  * @param    
  * @retval
 **/
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
		//判断接受个数是不是1帧的大小
		if(usart3_rx_len == BLE_FRAME_NUM)
		{
			//将数据复制到BLE_Solve_Buff
			memcpy(BLE_Solve_Buff,BLE_Receive_Buff,BLE_Receive_Buff_Len);
			//将数据放到队列里
			xQueueSendFromISR(BLE_Message_Queue,BLE_Solve_Buff,&xHigherPriorityTaskWoken);//向队列中发送数据
			//如果需要的话进行一次任务切换
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			//将接受数据清零
			memset(BLE_Receive_Buff,0,BLE_Receive_Buff_Len);
		}	
		//重新设置DMA接受长度
		hdma_usart3_rx.Instance->CNDTR =  BLE_Receive_Buff_Len;
		//使能DMA
		__HAL_DMA_ENABLE(&hdma_usart3_rx); 
		
    }
}

/**
  * @brief   串口DMA发送中断处理 ，有bug，未解决
  * @param    
  * @retval
 **/
void DMA1_Channel2_IRQHandler(void)
{	
	if(__HAL_DMA_GET_FLAG(&hdma_usart3_tx,DMA_FLAG_TC2))
	{
		__HAL_DMA_CLEAR_FLAG(&hdma_usart3_tx,DMA_FLAG_TC2);
		HAL_UART_DMAStop(&huart3);      //传输完成以后关闭串口DMA
	}	
}