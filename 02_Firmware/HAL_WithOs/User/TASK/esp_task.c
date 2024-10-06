#include "esp_task.h"

#include "esp01.h"
#include "usart.h"
#include "bsp_key.h" 
#include "control_task.h"
 
#define  ESP_Receive_Buff_Len     200
uint8_t  ESP_Receive_Buff[ESP_Receive_Buff_Len]={0};

static  void ESP8266_TCP_AP_SET(void);
static  void usart2_send_str(char *str,uint16_t strlen);
static  void extractData(const char* esp8266_buf,char* esp8266_data) ;

static  uint8_t esp_receive_flag = 0;

void esp_task(void const * argument)
{
	uint8_t keyValue = 0;
	char esp8266_buf[40];
	char esp8266_data[40];
	 
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart2, ESP_Receive_Buff, ESP_Receive_Buff_Len);
 	ESP8266_TCP_AP_SET();
	
	while(1)
	{
//		keyValue = KeyScan(SINGLE_SCAN);
//		if(KEY1_VALUE == keyValue){
//			 ESP8266_Send_Str("hello 1",0,sizeof("hello 1"));
//		}else if(KEY2_VALUE == keyValue){
//			 ESP8266_Send_Str("hello 2",0,sizeof("hello 2"));
//		}else{

//		}
		 
		if(esp_receive_flag == 1)
		{
			esp_receive_flag = 0;
//			memcpy(usart_send,ESP8266_Fram_Record_Struct.Data_RX_BUF,30);
//			printf("%s\r\n", ESP8266_Fram_Record_Struct.Data_RX_BUF);
//			ESP8266_Send_Str((char*)usart_send,0,sizeof(usart_send));
			
			strcpy(esp8266_buf, ESP8266_Fram_Record_Struct.Data_RX_BUF);
			extractData(esp8266_buf,esp8266_data);
			if(esp8266_data[0] == 'w')
				set_car_speed(15);
			else if(esp8266_data[0] == 's')
				set_car_speed(-15);
			else if(esp8266_data[0] == 'x')
				set_car_speed(0);
//			printf("���ݲ���2:%c\r\n", esp8266_data[0]);
 
		}
		vTaskDelay(5);
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
  * @brief   ����2��������
  * @param   str Ҫ���͵����� strlen Ҫ���͵����ݳ���
  * @retval   
 **/
void usart2_send_str(char *str,uint16_t strlen)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)str , strlen ,10 );
}

void ESP8266_TCP_AP_SET(void)
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
		
		if(esp_config_mode==1)
		{
			//��������Ǵ���ATָ���ʱ�򣬺���һ֡�����ݴ���������
			if(send_count_last == send_count )
			{
				strcat((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,(char*)ESP_Receive_Buff );
				ESP8266_Fram_Record_Struct.InfBit.FramLength += usart2_rx_len;
			}else
			{
				memcpy((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,(char*)ESP_Receive_Buff,ESP_Receive_Buff_Len);
				ESP8266_Fram_Record_Struct.InfBit.FramLength = usart2_rx_len; 
				send_count_last = send_count;
			}
		}else
		{
			//�������������ͨ�ŵ�ʱ���������ȡ����
			memcpy((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,(char*)ESP_Receive_Buff,ESP_Receive_Buff_Len);
			ESP8266_Fram_Record_Struct.InfBit.FramLength = usart2_rx_len; 
			esp_receive_flag = 1;
		}
		memset(ESP_Receive_Buff,0,ESP_Receive_Buff_Len);
				
		hdma_usart2_rx.Instance->CNDTR =  ESP_Receive_Buff_Len;
		__HAL_DMA_ENABLE(&hdma_usart2_rx); 
		
    }
}

void DMA1_Channel7_IRQHandler(void)
{	
	if(__HAL_DMA_GET_FLAG(&hdma_usart2_tx,DMA_FLAG_TC7))
	{
		__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx,DMA_FLAG_TC7);
		HAL_UART_DMAStop(&huart2);      //��������Ժ�رմ���DMA
	}	
}
 