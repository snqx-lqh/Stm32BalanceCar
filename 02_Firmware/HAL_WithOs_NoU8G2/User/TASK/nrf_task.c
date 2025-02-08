/**
  ******************************************************************************
  * @file    nrf_task.c
  * @author  少年潜行(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   这一部分是NRF遥控器的接受任务，我主要使用这一部分来进行遥控器控制小车移动。
			 这部分的主要方案是通过SPI检测并读取NRF模块的信息，并处理接受到的数据。然后在
			 控制任务中，有根据该协议帧执行的任务。
			 遥控器相关信息查看我的另外一个项目：
			 https://github.com/snqx-lqh/Stm32RemoteControl
			 NRF驱动使用的是软件SPI，软件SPI使用函数指针的方式定义接口。
			 NRF的API使用的是正点原子开源的代码。
			 NRF协议帧的定义
			 ------------------------------------------
			 |   序号  |      信号     |      值      |
			 |   0-1   |      帧头     |    0xAA55    |
			 |    2    |      帧长     |    0x20      |
			 |   3-4   | 遥控器左x通道 |    0-4095    |
			 |   5-6   | 遥控器左y通道 |    0-4095    |
			 |   7-8   | 遥控器右x通道 |    0-4095    |
			 |   9-10  | 遥控器右y通道 |    0-4095    |
			 |    11   |     按键值    |    0-255     |
			 |   12-13 |     gyrox     | -32767-32768 |
			 |   14-15 |     gyroy     | -32767-32768 |
			 |   16-17 |     gyroz     | -32767-32768 |
			 |   18-19 |     accx      | -32767-32768 |
			 |   20-21 |     accy      | -32767-32768 |
			 |   22-23 |     accz      | -32767-32768 |
			 |   24-25 |    row 100倍  | -18000-18000 |
			 |   26-27 |   pitch 100倍 | -18000-18000 |
			 |   28-29 |    yaw 100倍  | -18000-18000 |
			 |   30-31 |     校验和    |   0-65535    |
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

int16_t check_nrf = 0;    //NRF初始化检验
uint8_t check_tx  = 0;    //NRF发送标志位
uint8_t nrf_receive[32] = {0}; //NRF接受数据暂存区

rc_data_t rc_data;    //遥控器数据存储区

void nrf24l01_tx(void);
void nrf_data_init(void);
void nrf_receive_data_unpack(void);

/**
  * @brief   NRF任务处理 
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
		if(NRF24L01_RxPacket(nrf_receive)==0)//一旦接收到信息,则显示出来.
		{
			nrf_receive_data_unpack();
		}else{
			nrf_data_init();
		}
		vTaskDelay(50);
	}
}

/**
  * @brief   获得遥控器数据指针 
  * @param    
  * @retval
 **/
rc_data_t *get_rc_data()
{
	return &rc_data;
}

/**
  * @brief   将遥控器数据初始化
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
  * @brief   解包通过遥控器得到的数据 
  * @param    
  * @retval
 **/
void nrf_receive_data_unpack()
{
	uint16_t check_sum=0,check_sum_temp=0;
	if(nrf_receive[0] == 0xAA && nrf_receive[1] == 0x55)
	{
		//计算校验和
		for(int i=0;i<28;i++)
		{
			check_sum += nrf_receive[i+2];
		}
		check_sum_temp = (nrf_receive[30]<<8) + nrf_receive[31];
		if(check_sum == check_sum_temp)
		{
			//遥控器通道值
			rc_data.lx_value = (nrf_receive[3]<<8) + nrf_receive[4];
			rc_data.ly_value = (nrf_receive[5]<<8) + nrf_receive[6];
			rc_data.rx_value = (nrf_receive[7]<<8) + nrf_receive[8];
			rc_data.ry_value = (nrf_receive[9]<<8) + nrf_receive[10];
			//按键值
			rc_data.key_value = nrf_receive[11];
			//陀螺仪原始值
			rc_data.gyro[0] = (nrf_receive[12]<<8) + nrf_receive[13];
			rc_data.gyro[1] = (nrf_receive[14]<<8) + nrf_receive[15];
			rc_data.gyro[2] = (nrf_receive[16]<<8) + nrf_receive[17];
			//加速度计原始值
			rc_data.acc[0] = (nrf_receive[18]<<8) + nrf_receive[19];
			rc_data.acc[1] = (nrf_receive[20]<<8) + nrf_receive[21];
			rc_data.acc[2] = (nrf_receive[22]<<8) + nrf_receive[23];
			//角度原始值 
			rc_data.angle[0] = 0.01f*( (int16_t)((nrf_receive[24]<<8) + nrf_receive[25]) );
			rc_data.angle[1] = 0.01f*( (int16_t)((nrf_receive[26]<<8) + nrf_receive[27]) );
			rc_data.angle[2] = 0.01f*( (int16_t)((nrf_receive[28]<<8) + nrf_receive[29]) );
		}
	}
}

/**
  * @brief   遥控器发送处理 ，暂时没做处理
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
