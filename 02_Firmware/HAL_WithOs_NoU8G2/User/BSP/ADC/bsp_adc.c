#include "bsp_adc.h"

#include "adc.h"

static uint16_t adc_value[4] = {0};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	
}

void adc_convert()
{
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_value,4);
}

float get_power_value()
{
	return adc_value[3]/4096.0*3.3f*11;;
}