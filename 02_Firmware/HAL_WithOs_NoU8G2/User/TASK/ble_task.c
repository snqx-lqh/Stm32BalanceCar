/**
  ******************************************************************************
  * @file    ble_task.c
  * @author  ����Ǳ��(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   ��һ�����������������Ľ�����������Ҫʹ����һ����������������λ��
			 ����С����PID
			 �ⲿ�ֵ���Ҫ������ͨ�����ڿ����жϼ�DMA�����������ݣ����ܵ������ݷ���
			 ����õ�����������Ϣ���У�Ȼ���������м�⵽�����������ݣ��ʹ�����ص�
			 ���ݡ�
			 ����Э��֡�Ķ���
			 ����֡��  ����֡��Ҫ����PID����������ֵ����Ҫ����ʵ�������������ȡ��
			 ------------------------------------------------------------------------------------------
			 |֡ͷ|�Ƕ�KP��8λ|�Ƕ�KP��8λ|�Ƕ�KD��8λ|�Ƕ�KD��8λ|�ٶ�KP��8λ|�ٶ�KP��8λ|У���|֡β|
			 |  0 |      1    |      2    |      3    |      4    |      5    |      6    |   7  |  8 |
			 |0xA5|      -    |      -    |      -    |      -    |      -    |      -    |   -  |0X5A|
			 ------------------------------------------------------------------------------------------
			 ����֡��  ����֡��Ҫ�鿴�Ƕȱ仯
			 ------------------------------------------------------------------------------------------
			 |֡ͷ|�Ƕȵ�8λ|�Ƕȸ�8λ|У���|֡β|
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

#define  BLE_Receive_Buff_Len     20 //���ý������ݵ���󻺳�ֵ
#define  BLE_Q_NUM                4  //���ö��еĳ���
#define  BLE_FRAME_NUM            9  //����1֡��������

uint8_t  BLE_Receive_Buff[BLE_Receive_Buff_Len]={0};  //�������ݽ�������
uint8_t  BLE_Solve_Buff[BLE_Receive_Buff_Len]  ={0};    //�����ܵ������ݸ��Ƶ����飬����û����һ��Ҳûɶ����

static QueueHandle_t BLE_Message_Queue; //�����������ݻ������

/**
  * @brief   ���������� 
  * @param    
  * @retval
 **/
void ble_task(void const * argument)
{
	uint8_t  Receive_Buff[BLE_Receive_Buff_Len]={0}; //������ȡ�����ݵ���ʱ��Ż���
	uint8_t  Send_Buff[BLE_Receive_Buff_Len]={0};    //�������ݵĻ�������
	
	int16_t  angle = 0;      //С���Ƕ�ֵ
	float    angleKpSet = 0,angleKdSet = 0,speedKpSet = 0;   //С��PID����ֵ
	uint8_t  check_sum = 0;  //У���ֵ
	const balance_car_t *balance_car_temp = get_car_statu(); //���ƽ�⳵�ı�����ַ
	
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);    //�򿪴���3�����ж�
	HAL_UART_Receive_DMA(&huart3, BLE_Receive_Buff, BLE_Receive_Buff_Len); //����DMA��BUFF�ͳ���
	BLE_Message_Queue = xQueueCreate(BLE_Q_NUM,BLE_Receive_Buff_Len); //�����������ݶ���
	
	vTaskDelay(1000);
	while(1)
	{
		if(BLE_Message_Queue!=NULL)
        {
            if(xQueueReceive(BLE_Message_Queue,Receive_Buff,portMAX_DELAY))//����������������� 
            {
				//�����������յ�������
				if(Receive_Buff[0] == 0XA5 && Receive_Buff[8] == 0X5A)
				{
					//У��ͼ��
					check_sum = Receive_Buff[1] + Receive_Buff[2] + Receive_Buff[3] + Receive_Buff[4] + Receive_Buff[5] + Receive_Buff[6];
					if(check_sum == Receive_Buff[7])
					{
						//�������ݣ�����KD�������������ǰ������Ŵ��˵ģ���Ҫ����ȥ
						angleKpSet = (float)((Receive_Buff[2] << 8) + Receive_Buff[1]);
						angleKdSet = (float)((Receive_Buff[4] << 8) + Receive_Buff[3]);
						speedKpSet = (float)((Receive_Buff[6] << 8) + Receive_Buff[5]);
						angleKpSet = -angleKpSet;
						angleKdSet = -angleKdSet/100;
						speedKpSet = -speedKpSet;
						//����С��PID����
						set_car_pid(angleKpSet,angleKdSet,speedKpSet);
					}else
					{
						
					}
				}	
				//ͨ����������ƽ�⳵�Ƕ�����
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
  * @brief   ���������жϴ��� 
  * @param    
  * @retval
 **/
void USART3_IRQHandler(void)
{
	uint8_t usart3_rx_len = 0;
	BaseType_t xHigherPriorityTaskWoken;
    //�������˴��ڿ����ж�
    if((__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET)) 
    {
		__HAL_UART_CLEAR_IDLEFLAG(&huart3); //������б�־
		__HAL_DMA_DISABLE(&hdma_usart3_rx); //��ֹͣDMA����ͣ����
		//���ո������ڽ��ջ������ܴ�С��ʣ�����
		usart3_rx_len = BLE_Receive_Buff_Len - (__HAL_DMA_GET_COUNTER(&hdma_usart3_rx)); 
		//�жϽ��ܸ����ǲ���1֡�Ĵ�С
		if(usart3_rx_len == BLE_FRAME_NUM)
		{
			//�����ݸ��Ƶ�BLE_Solve_Buff
			memcpy(BLE_Solve_Buff,BLE_Receive_Buff,BLE_Receive_Buff_Len);
			//�����ݷŵ�������
			xQueueSendFromISR(BLE_Message_Queue,BLE_Solve_Buff,&xHigherPriorityTaskWoken);//������з�������
			//�����Ҫ�Ļ�����һ�������л�
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			//��������������
			memset(BLE_Receive_Buff,0,BLE_Receive_Buff_Len);
		}	
		//��������DMA���ܳ���
		hdma_usart3_rx.Instance->CNDTR =  BLE_Receive_Buff_Len;
		//ʹ��DMA
		__HAL_DMA_ENABLE(&hdma_usart3_rx); 
		
    }
}

/**
  * @brief   ����DMA�����жϴ��� ����bug��δ���
  * @param    
  * @retval
 **/
void DMA1_Channel2_IRQHandler(void)
{	
	if(__HAL_DMA_GET_FLAG(&hdma_usart3_tx,DMA_FLAG_TC2))
	{
		__HAL_DMA_CLEAR_FLAG(&hdma_usart3_tx,DMA_FLAG_TC2);
		HAL_UART_DMAStop(&huart3);      //��������Ժ�رմ���DMA
	}	
}