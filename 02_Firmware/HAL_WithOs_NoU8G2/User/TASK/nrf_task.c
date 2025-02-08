/**
  ******************************************************************************
  * @file    nrf_task.c
  * @author  ����Ǳ��(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   ��һ������NRFң�����Ľ�����������Ҫʹ����һ����������ң��������С���ƶ���
			 �ⲿ�ֵ���Ҫ������ͨ��SPI��Ⲣ��ȡNRFģ�����Ϣ����������ܵ������ݡ�Ȼ����
			 ���������У��и��ݸ�Э��ִ֡�е�����
			 ң���������Ϣ�鿴�ҵ�����һ����Ŀ��
			 https://github.com/snqx-lqh/Stm32RemoteControl
			 NRF����ʹ�õ������SPI�����SPIʹ�ú���ָ��ķ�ʽ����ӿڡ�
			 NRF��APIʹ�õ�������ԭ�ӿ�Դ�Ĵ��롣
			 NRFЭ��֡�Ķ���
			 ------------------------------------------
			 |   ���  |      �ź�     |      ֵ      |
			 |   0-1   |      ֡ͷ     |    0xAA55    |
			 |    2    |      ֡��     |    0x20      |
			 |   3-4   | ң������xͨ�� |    0-4095    |
			 |   5-6   | ң������yͨ�� |    0-4095    |
			 |   7-8   | ң������xͨ�� |    0-4095    |
			 |   9-10  | ң������yͨ�� |    0-4095    |
			 |    11   |     ����ֵ    |    0-255     |
			 |   12-13 |     gyrox     | -32767-32768 |
			 |   14-15 |     gyroy     | -32767-32768 |
			 |   16-17 |     gyroz     | -32767-32768 |
			 |   18-19 |     accx      | -32767-32768 |
			 |   20-21 |     accy      | -32767-32768 |
			 |   22-23 |     accz      | -32767-32768 |
			 |   24-25 |    row 100��  | -18000-18000 |
			 |   26-27 |   pitch 100�� | -18000-18000 |
			 |   28-29 |    yaw 100��  | -18000-18000 |
			 |   30-31 |     У���    |   0-65535    |
			 ------------------------------------------
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; Copyright {Year} LQH,China</center></h2>
  ******************************************************************************
  */
#include "nrf_task.h"

#include "nrf24L01.h"

int16_t check_nrf = 0;    //NRF��ʼ������
uint8_t check_tx  = 0;    //NRF���ͱ�־λ
uint8_t nrf_receive[32] = {0}; //NRF���������ݴ���

rc_data_t rc_data;    //ң�������ݴ洢��

void nrf24l01_tx(void);
void nrf_data_init(void);
void nrf_receive_data_unpack(void);

/**
  * @brief   NRF������ 
  * @param    
  * @retval
 **/
void nrf_task(void const * argument)
{
	NRF24L01_Init();
	check_nrf = NRF24L01_Check();
	NRF24L01_RX_Mode();
	while(1)
	{
		if(NRF24L01_RxPacket(nrf_receive)==0)//һ�����յ���Ϣ,����ʾ����.
		{
			nrf_receive_data_unpack();
		}else{
			nrf_data_init();
		}
		vTaskDelay(50);
	}
}

/**
  * @brief   ���ң��������ָ�� 
  * @param    
  * @retval
 **/
rc_data_t *get_rc_data()
{
	return &rc_data;
}

/**
  * @brief   ��ң�������ݳ�ʼ��
  * @param    
  * @retval
 **/
void nrf_data_init()
{
	rc_data.lx_value =  2048;
	rc_data.ly_value =  2048;
	rc_data.rx_value =  2048;
	rc_data.ry_value =  2048;
}

/**
  * @brief   ���ͨ��ң�����õ������� 
  * @param    
  * @retval
 **/
void nrf_receive_data_unpack()
{
	uint16_t check_sum=0,check_sum_temp=0;
	if(nrf_receive[0] == 0xAA && nrf_receive[1] == 0x55)
	{
		//����У���
		for(int i=0;i<28;i++)
		{
			check_sum += nrf_receive[i+2];
		}
		check_sum_temp = (nrf_receive[30]<<8) + nrf_receive[31];
		if(check_sum == check_sum_temp)
		{
			//ң����ͨ��ֵ
			rc_data.lx_value = (nrf_receive[3]<<8) + nrf_receive[4];
			rc_data.ly_value = (nrf_receive[5]<<8) + nrf_receive[6];
			rc_data.rx_value = (nrf_receive[7]<<8) + nrf_receive[8];
			rc_data.ry_value = (nrf_receive[9]<<8) + nrf_receive[10];
			//����ֵ
			rc_data.key_value = nrf_receive[11];
			//������ԭʼֵ
			rc_data.gyro[0] = (nrf_receive[12]<<8) + nrf_receive[13];
			rc_data.gyro[1] = (nrf_receive[14]<<8) + nrf_receive[15];
			rc_data.gyro[2] = (nrf_receive[16]<<8) + nrf_receive[17];
			//���ٶȼ�ԭʼֵ
			rc_data.acc[0] = (nrf_receive[18]<<8) + nrf_receive[19];
			rc_data.acc[1] = (nrf_receive[20]<<8) + nrf_receive[21];
			rc_data.acc[2] = (nrf_receive[22]<<8) + nrf_receive[23];
			//�Ƕ�ԭʼֵ 
			rc_data.angle[0] = 0.01f*( (int16_t)((nrf_receive[24]<<8) + nrf_receive[25]) );
			rc_data.angle[1] = 0.01f*( (int16_t)((nrf_receive[26]<<8) + nrf_receive[27]) );
			rc_data.angle[2] = 0.01f*( (int16_t)((nrf_receive[28]<<8) + nrf_receive[29]) );
		}
	}
}

/**
  * @brief   ң�������ʹ��� ����ʱû������
  * @param    
  * @retval
 **/
void nrf24l01_tx()
{
	uint8_t nrf_tx_buff[32] = {0};

	NRF24L01_TX_Mode( );
	check_tx = NRF24L01_TxPacket(nrf_tx_buff);
	NRF24L01_RX_Mode();
}
