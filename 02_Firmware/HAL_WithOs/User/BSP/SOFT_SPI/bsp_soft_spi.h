#ifndef _BSP_SOFT_SPI_H
#define _BSP_SOFT_SPI_H

#include "main.h"
#include <stdint.h>

typedef struct soft_spi_operations{
	void (*spi_init)(void);
	void (*set_spi_sck_level)(unsigned char level);
	void (*set_spi_mosi_level)(unsigned char level);
	uint8_t (*spi_miso_read)(void);
    uint8_t  spi_mode;	
}soft_spi_t;

uint8_t soft_spi_init(soft_spi_t *soft_spi);
uint8_t soft_spi_read_write_byte(soft_spi_t *soft_spi,uint8_t write_dat);

uint8_t SPI_ReadWriteByte_Mode0(soft_spi_t *soft_spi,uint8_t write_dat );
uint8_t SPI_ReadWriteByte_Mode1(soft_spi_t *soft_spi,uint8_t write_dat );
uint8_t SPI_ReadWriteByte_Mode2(soft_spi_t *soft_spi,uint8_t write_dat );
uint8_t SPI_ReadWriteByte_Mode3(soft_spi_t *soft_spi,uint8_t write_dat );

#endif

