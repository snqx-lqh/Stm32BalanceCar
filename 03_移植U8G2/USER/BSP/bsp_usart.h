#ifndef _BSP_USART_H
#define _BSP_USART_H

#include "main.h"

/*
串口号    没有重映像    部分重映像   全部重映像
1			A9/A10	                    B6/B7
2           A2/A3                       D5/D6
3           B10/B11       C10/C11       D8/D9
4           C10/C11
5           C12/D2
*/

void bsp_usart1_init(u32 bound);
void bsp_usart3_init(u32 bound);

//DEBUG串口接收中断函数，中断中已经清除标志位
void DebugUsartMain(void);


#endif

