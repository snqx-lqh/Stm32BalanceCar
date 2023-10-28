#ifndef _BSP_ADC_H
#define _BSP_ADC_H

#include "main.h"

void bsp_adc_init(void);
u16 adc_get_value(u8 ch);

#endif

