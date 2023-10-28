#ifndef _BSP_KEY_H
#define _BSP_KEY_H

#include "main.h"

void bsp_key_init(void);
uint8_t get_key_value(void);

#define KEY_OK      PAin(15)

#endif

