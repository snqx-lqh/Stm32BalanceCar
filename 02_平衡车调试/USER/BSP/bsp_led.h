#ifndef _BSP_LED_H
#define _BSP_LED_H

#include "main.h"

void bsp_led_init(void);
void led_board_on(void);
void led_board_off(void);
void led_red_on(void);
void led_red_off(void);

#define LED_BOARD PCout(13)
#define LED_RED   PAout(12)

#endif

