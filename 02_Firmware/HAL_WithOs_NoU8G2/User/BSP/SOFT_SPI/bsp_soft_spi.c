/**
  ******************************************************************************
  * @file    bsp_soft_spi.c 
  * @author  少年潜行(snqx-lqh)
  * @version V
  * @date    2024-03-01
  * @brief   软件SPI模块，使用模块化编程，用户使用的时候需要重写soft_spi_t结构体中
			 的相关函数
			 void (*spi_sck_high)(void);            
			 void (*spi_sck_low)(void);
			 void (*spi_mosi_high)(void);
			 void (*spi_mosi_low)(void);
			 uint8_t (*spi_miso_read)(void);
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; Copyright 2024 LQH,China</center></h2>
  ******************************************************************************
  */
#include "bsp_soft_spi.h"

uint8_t soft_spi_init(soft_spi_t *soft_spi)
{
	if(soft_spi == NULL)
		return 1;
	soft_spi->spi_init();
	return 0;
}

uint8_t soft_spi_read_write_byte(soft_spi_t *soft_spi,uint8_t write_dat)
{
	uint8_t i=0, read_dat=0, read_temp=0;
	if(soft_spi == NULL)
		return 1;
	if(soft_spi->spi_mode == 0)
	{
		for( i = 0; i < 8; i++ )
		{
			if( write_dat & 0x80 )
				soft_spi->set_spi_mosi_level(1);  
			else                    
				soft_spi->set_spi_mosi_level(0);  
			write_dat <<= 1;
			soft_spi->set_spi_sck_level(1); 
			read_dat <<= 1;  
			read_temp = soft_spi->spi_miso_read();
			if( read_temp ) 
				read_dat |= 0x01; 
			soft_spi->set_spi_sck_level(0); 
		}	
	}else if(soft_spi->spi_mode == 1)
	{
		for(i=0;i<8;i++)     
		{
			soft_spi->set_spi_sck_level(1);      
			if(write_dat&0x80)
				soft_spi->set_spi_mosi_level(1);   
			else      
				soft_spi->set_spi_mosi_level(0);    
			write_dat <<= 1;     
			soft_spi->set_spi_sck_level(0);      
			read_dat <<= 1;    
			read_temp = soft_spi->spi_miso_read();
			if( read_temp ) 
				read_dat |= 0x01;    
		}
	}else if(soft_spi->spi_mode == 2)
	{
		for(i=0;i<8;i++)    
		{
			if(write_dat&0x80)
				soft_spi->set_spi_mosi_level(1);  
			else      
				soft_spi->set_spi_mosi_level(0);   
			write_dat <<= 1;     
			soft_spi->set_spi_sck_level(0);     
			read_dat <<= 1;     
			read_temp = soft_spi->spi_miso_read();
			if( read_temp ) 
				read_dat |= 0x01;      
			soft_spi->set_spi_sck_level(1);  
		}
	}else if(soft_spi->spi_mode == 3)
	{
		for( i = 0; i < 8; i++ )
		{
			soft_spi->set_spi_sck_level(0);
			if( write_dat & 0x80 )
				soft_spi->set_spi_mosi_level(1); 
			else                    
				soft_spi->set_spi_mosi_level(0);
			write_dat <<= 1;
			soft_spi->set_spi_sck_level(1);
			read_dat <<= 1;  
			read_temp = soft_spi->spi_miso_read();
			if( read_temp ) 
				read_dat |= 0x01; 
		}
	}
	return read_dat;
}


 
