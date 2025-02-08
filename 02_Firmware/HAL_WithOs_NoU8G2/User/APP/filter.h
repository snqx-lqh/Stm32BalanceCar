#ifndef _FILTER_H
#define _FILTER_H

#include "main.h"

typedef struct {
    double Q_angle;
    double Q_bias;
    double R_measure;
    double angle;
    double bias;
    double P[2][2];
} Kalman_t;

extern Kalman_t KalmanX,KalmanY;
extern float angle;
void FirstOrderLowPassFilter(float angle_m, float gyro_m, float dt);
double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt);
 

#endif

