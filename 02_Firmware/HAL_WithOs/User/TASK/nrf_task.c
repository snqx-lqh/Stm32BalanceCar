#include "nrf_task.h"

#include "nrf24L01.h"

int16_t check_nrf = 0;
uint8_t check_tx  = 0;
uint8_t nrf_test[32] = {0};

void nrf24l01_tx(void);

void nrf_task(void const * argument)
{
	NRF24L01_Init();
	check_nrf = NRF24L01_Check( );
	NRF24L01_RX_Mode( );
	while(1)
	{
		if(NRF24L01_RxPacket(nrf_test)==0)//一旦接收到信息,则显示出来.
		{
			
		}
		vTaskDelay(100);
	}
}

void nrf24l01_tx()
{
	uint8_t nrf_tx_buff[32] = {0};

	nrf_tx_buff[0] = 0xA3;
	NRF24L01_TX_Mode( );
	check_tx = NRF24L01_TxPacket(nrf_tx_buff);
}
