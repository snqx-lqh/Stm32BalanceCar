#ifndef _BSP_DELAY_H
#define _BSP_DELAY_H

#include "stm32f10x.h"

void delay_init(void);
void delay_us(u32 nus);
void delay_ms(u16 nms);

#endif
