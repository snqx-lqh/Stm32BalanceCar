#ifndef _BSP_KEY_H
#define _BSP_KEY_H

#include "main.h"
#include "gpio.h"

typedef enum{
	SINGLE_SCAN = 0,
	CONTINUOUS_SCAN,
}key_scan_mode_t;

#define KEY1  HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)
#define KEY2  HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)

#define KEY1_VALUE 1
#define KEY2_VALUE 2

uint8_t KeyScan(const key_scan_mode_t key_scan_mode);

#endif
