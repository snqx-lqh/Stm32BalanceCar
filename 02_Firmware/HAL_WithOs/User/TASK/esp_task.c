/**
  ******************************************************************************
  * @file    esp_task.c
  * @author  少年潜行(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   这一部分是ESP8266的接受任务，我主要使用这一部分来进行遥控器控制小车移动。
			 通过手机APP和ESP8266相连，然后手机发送指令信息控制小车移动
			 ESP8266配置的是AP模式，使用手机去连接。
			 手机上的上下左右和中间分别会给ESP发送F B L R S
			 这一部分的代码和ESP01.c里面的解耦做的不是很好，后续可能会改
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; Copyright {Year} LQH,China</center></h2>
  ******************************************************************************
  */
#include "esp_task.h"

#include "esp01.h"
#include "usart.h"
#include "bsp_key.h" 
#include "control_task.h"
 
#define  ESP_Receive_Buff_Len     200
uint8_t  ESP_Receive_Buff[ESP_Receive_Buff_Len]={0};  //ESP8266接受的数据缓冲区

static  void ESP8266_TCP_AP_SET(void);
static  void extractData(const char* esp8266_buf,char* esp8266_data) ;

static TaskHandle_t ESP_task_local_handler;

void esp_task(void const * argument)
{
	uint8_t keyValue = 0;
	char esp8266_buf[20];
	char esp8266_data[10];
	 
	//获取当前任务的任务句柄
    ESP_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));
	//设置串口相关配置
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart2, ESP_Receive_Buff, ESP_Receive_Buff_Len);
	//设置ESP8266为AP模式
 	ESP8266_TCP_AP_SET();
	
	while(1)
	{
		//等待中断中的任务通知
		while (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) != pdPASS)
        {
        } 

		strcpy(esp8266_buf, ESP8266_Fram_Record_Struct.Data_RX_BUF);
		extractData(esp8266_buf,esp8266_data);
		if(esp8266_data[0] == 'F')
			set_car_speed(15);
		else if(esp8266_data[0] == 'B')
			set_car_speed(-15);
		else if(esp8266_data[0] == 'L')
			set_car_turn(15);
		else if(esp8266_data[0] == 'R')
			set_car_turn(-15);
		else if(esp8266_data[0] == 'S')
		{
			set_car_speed(0);
			set_car_turn(0);
		}
	}
}

/**
  * @brief   将传回来的数据进行解析，把数据部分取出来
  * @param   esp8266_buf 接收到的全部数据  esp8266_data +IPD后面的结果数据
  * @retval   
 **/
static  void extractData(const char* esp8266_buf,char* esp8266_data) 
{
    int port;
    int numBytes;

    if (strstr(esp8266_buf, "+IPD")) {
        sscanf(esp8266_buf, "\r\n+IPD,%d,%d:%s", &port, &numBytes, esp8266_data);
		//printf("+IPD,%d,%d:%s", port, numBytes, esp8266_data);
    }
}

/**
  * @brief   配置ESP8266为AP模式
  * @param    
  * @retval   
 **/
static void ESP8266_TCP_AP_SET(void)
{
    bool res;
	printf("Set ESP8266\r\n");
    res = ESP8266_Net_Mode_Choose(AP);
	if(res == false) printf("\r\nerror 1"); 
    res = ESP8266_Enable_MultipleId ( ENABLE );
	if(res == false) printf("\r\nerror 2"); 
	res = ESP8266_Create_Tcp_server();
	if(res == false) printf("\r\nerror 3"); 
	printf("\r\n配置完成"); 
}

/**
  * @brief   ESP8266串口中断处理 
  * @param    
  * @retval
 **/
void USART2_IRQHandler( void )
{   
	static uint16_t send_count_last = 0;
	uint8_t usart2_rx_len = 0;
    //当触发了串口空闲中断
    if((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)) 
    {
		__HAL_UART_CLEAR_IDLEFLAG(&huart2); //清除空闲标志
		__HAL_DMA_DISABLE(&hdma_usart2_rx); //先停止DMA，暂停接收
		//这里应注意数据接收不要大于 USART_DMA_RX_BUFFER_MAXIMUM
		usart2_rx_len = ESP_Receive_Buff_Len - (__HAL_DMA_GET_COUNTER(&hdma_usart2_rx)); //接收个数等于接收缓冲区总大小减剩余计数
		
		if(get_esp_config_mode() == 1)
		{
			//这种想的是处理AT指令的时候，害怕一帧传数据传不回来完,就把数据进行拼接
			if(send_count_last == get_send_count() )
			{
				strcat((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,(char*)ESP_Receive_Buff );
				ESP8266_Fram_Record_Struct.InfBit.FramLength += usart2_rx_len;
			}else
			{
				memcpy((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,(char*)ESP_Receive_Buff,ESP_Receive_Buff_Len);
				ESP8266_Fram_Record_Struct.InfBit.FramLength = usart2_rx_len; 
				send_count_last = get_send_count();
			}
		}else
		{
			//这里想的是正常通信的时候就正常获取数据
			memcpy((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,(char*)ESP_Receive_Buff,ESP_Receive_Buff_Len);
			ESP8266_Fram_Record_Struct.InfBit.FramLength = usart2_rx_len; 
			//任务通知唤醒，通知ESP接受任务 
			if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
			{
				static BaseType_t xHigherPriorityTaskWoken;
				vTaskNotifyGiveFromISR(ESP_task_local_handler, &xHigherPriorityTaskWoken);
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
		//设置接受缓存为0
		memset(ESP_Receive_Buff,0,ESP_Receive_Buff_Len);
		//设置DMA接受数据长度和使能DMA
		hdma_usart2_rx.Instance->CNDTR =  ESP_Receive_Buff_Len;
		__HAL_DMA_ENABLE(&hdma_usart2_rx); 
    }
}

/**
  * @brief   ESP8266串口发送DMA中断处理，有BUG 
  * @param    
  * @retval
 **/
void DMA1_Channel7_IRQHandler(void)
{	
	if(__HAL_DMA_GET_FLAG(&hdma_usart2_tx,DMA_FLAG_TC7))
	{
		__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx,DMA_FLAG_TC7);
		HAL_UART_DMAStop(&huart2);      //传输完成以后关闭串口DMA
	}	
}
 