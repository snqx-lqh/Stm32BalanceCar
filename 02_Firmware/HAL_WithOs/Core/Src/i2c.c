/**
  ******************************************************************************
  * File Name          : I2C.c
  * Description        : This file provides code for the configuration
  *                      of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_I2C1_ENABLE();

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

static SemaphoreHandle_t MutexSemaphore;

void create_i2c_mutex(void)
{
	MutexSemaphore=xSemaphoreCreateMutex();
}

void u_i2c1_write_byte(unsigned char add,unsigned char reg,unsigned char *data)
{
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		// 等待互斥量，确保在写入共享数据时不会被其他任务中断
		if (xSemaphoreTake(MutexSemaphore, portMAX_DELAY) == pdTRUE) {
			HAL_I2C_Mem_Write(&hi2c1, (add<<1), reg, I2C_MEMADD_SIZE_8BIT, data,1,HAL_MAX_DELAY);
			// 释放互斥量
			xSemaphoreGive(MutexSemaphore);
		}
	}else{
		HAL_I2C_Mem_Write(&hi2c1, (add<<1), reg, I2C_MEMADD_SIZE_8BIT, data,1,HAL_MAX_DELAY);
	}
}

void u_i2c1_write_bytes(unsigned char add,unsigned char reg,unsigned char *data,unsigned char len)
{	
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		// 等待互斥量，确保在写入共享数据时不会被其他任务中断
		if (xSemaphoreTake(MutexSemaphore, portMAX_DELAY) == pdTRUE) {
			HAL_I2C_Mem_Write(&hi2c1, (add<<1), reg, I2C_MEMADD_SIZE_8BIT, data,len,HAL_MAX_DELAY);
			// 释放互斥量
			xSemaphoreGive(MutexSemaphore);
		}
	}else{
		HAL_I2C_Mem_Write(&hi2c1, (add<<1), reg, I2C_MEMADD_SIZE_8BIT, data,len,HAL_MAX_DELAY);
	}
}

void u_i2c1_read_byte(unsigned char add,unsigned char reg,unsigned char *data)
{		
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		// 等待互斥量，确保在写入共享数据时不会被其他任务中断
		if (xSemaphoreTake(MutexSemaphore, portMAX_DELAY) == pdTRUE) {
			HAL_I2C_Mem_Read(&hi2c1, (add<<1), reg,I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
			// 释放互斥量
			xSemaphoreGive(MutexSemaphore);
		}
	}else{
		HAL_I2C_Mem_Read(&hi2c1, (add<<1), reg,I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
	}
}

void u_i2c1_read_bytes(unsigned char add,unsigned char reg,unsigned char *data,unsigned char len)
{		
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		// 等待互斥量，确保在写入共享数据时不会被其他任务中断
		if (xSemaphoreTake(MutexSemaphore, portMAX_DELAY) == pdTRUE) {
			HAL_I2C_Mem_Read(&hi2c1, (add<<1), reg,I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY);  
			// 释放互斥量
			xSemaphoreGive(MutexSemaphore);
		}
	}else{
		HAL_I2C_Mem_Read(&hi2c1, (add<<1), reg,I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY);  
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
