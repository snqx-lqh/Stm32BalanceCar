#ifndef _HC_SR04_H
#define _HC_SR04_H

#include "main.h"

void trig_send_pluse(void);
void hc_sr04_init(void);
uint16_t get_hcsr04_count(void); 
uint16_t get_hcsr04_mm(void);

#endif
