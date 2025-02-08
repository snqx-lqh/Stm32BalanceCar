/**
  ******************************************************************************
  * @file    esp_task.c
  * @author  ����Ǳ��(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   ��һ������ESP8266�Ľ�����������Ҫʹ����һ����������ң��������С���ƶ���
			 ͨ���ֻ�APP��ESP8266������Ȼ���ֻ�����ָ����Ϣ����С���ƶ�
			 ESP8266���õ���APģʽ��ʹ���ֻ�ȥ���ӡ�
			 �ֻ��ϵ��������Һ��м�ֱ���ESP����F B L R S
			 ��һ���ֵĴ����ESP01.c����Ľ������Ĳ��Ǻܺã��������ܻ��
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
uint8_t  ESP_Receive_Buff[ESP_Receive_Buff_Len]={0};  //ESP8266���ܵ����ݻ�����

static  void ESP8266_TCP_AP_SET(void);
static  void extractData(const char* esp8266_buf,char* esp8266_data) ;

static TaskHandle_t ESP_task_local_handler;

void esp_task(void const * argument)
{
	uint8_t keyValue = 0;
	char esp8266_buf[20];
	char esp8266_data[10];
	 
	//��ȡ��ǰ�����������
    ESP_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));
	//���ô����������
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart2, ESP_Receive_Buff, ESP_Receive_Buff_Len);
	//����ESP8266ΪAPģʽ
 	ESP8266_TCP_AP_SET();
	
	while(1)
	{
		//�ȴ��ж��е�����֪ͨ
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
  * @brief   �������������ݽ��н����������ݲ���ȡ����
  * @param   esp8266_buf ���յ���ȫ������  esp8266_data +IPD����Ľ������
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
  * @brief   ����ESP8266ΪAPģʽ
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
	printf("\r\n�������"); 
}

/**
  * @brief   ESP8266�����жϴ��� 
  * @param    
  * @retval
 **/
void USART2_IRQHandler( void )
{   
	static uint16_t send_count_last = 0;
	uint8_t usart2_rx_len = 0;
    //�������˴��ڿ����ж�
    if((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)) 
    {
		__HAL_UART_CLEAR_IDLEFLAG(&huart2); //������б�־
		__HAL_DMA_DISABLE(&hdma_usart2_rx); //��ֹͣDMA����ͣ����
		//����Ӧע�����ݽ��ղ�Ҫ���� USART_DMA_RX_BUFFER_MAXIMUM
		usart2_rx_len = ESP_Receive_Buff_Len - (__HAL_DMA_GET_COUNTER(&hdma_usart2_rx)); //���ո������ڽ��ջ������ܴ�С��ʣ�����
		
		if(get_esp_config_mode() == 1)
		{
			//��������Ǵ���ATָ���ʱ�򣬺���һ֡�����ݴ���������,�Ͱ����ݽ���ƴ��
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
			//�������������ͨ�ŵ�ʱ���������ȡ����
			memcpy((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,(char*)ESP_Receive_Buff,ESP_Receive_Buff_Len);
			ESP8266_Fram_Record_Struct.InfBit.FramLength = usart2_rx_len; 
			//����֪ͨ���ѣ�֪ͨESP�������� 
			if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
			{
				static BaseType_t xHigherPriorityTaskWoken;
				vTaskNotifyGiveFromISR(ESP_task_local_handler, &xHigherPriorityTaskWoken);
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
		//���ý��ܻ���Ϊ0
		memset(ESP_Receive_Buff,0,ESP_Receive_Buff_Len);
		//����DMA�������ݳ��Ⱥ�ʹ��DMA
		hdma_usart2_rx.Instance->CNDTR =  ESP_Receive_Buff_Len;
		__HAL_DMA_ENABLE(&hdma_usart2_rx); 
    }
}

/**
  * @brief   ESP8266���ڷ���DMA�жϴ�����BUG 
  * @param    
  * @retval
 **/
void DMA1_Channel7_IRQHandler(void)
{	
	if(__HAL_DMA_GET_FLAG(&hdma_usart2_tx,DMA_FLAG_TC7))
	{
		__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx,DMA_FLAG_TC7);
		HAL_UART_DMAStop(&huart2);      //��������Ժ�رմ���DMA
	}	
}
 