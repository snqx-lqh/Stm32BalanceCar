#ifndef _FILTER_H
#define _FILTER_H

#include "main.h"

extern float angle;

void FirstOrderLowPassFilter(float angle_m, float gyro_m);
void Kalman_Filter(float Accel,float Gyro);

#endif

