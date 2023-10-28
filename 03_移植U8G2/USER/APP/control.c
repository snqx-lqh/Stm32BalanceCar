#include "control.h"
#include "mpu6050.h"
#include "filter.h"
#include "math.h"
#include "bsp_motor.h"

void angle_control(void);
void speed_control(void);
void turn_control(void);

int16_t leftPwm=0,rightPwm=0;

float   angleAim=1;
int16_t anglePwm=0;
int16_t gyroBalance = 0;
float   angleKp = 720;//490
float   angleKd = 1.7;//14

int16_t leftEncode=0,rightEncode=0;
int16_t speedAim = 0;
int16_t speedNow = 0;
int16_t speedPwm = 0;
float   speedKp = -140;//80
float   speedKi = -0.7;//0.4

int16_t turnPwm = 0;
float   turnKp = 0;
float   turnKd = 0;

u8 Flag_Target = 0;

int car_control()
{
    Flag_Target=!Flag_Target;
    if(Flag_Target==1)                                                  //5ms读取一次陀螺仪和加速度计的值，更高的采样频率可以改善卡尔曼滤波和互补滤波的效果
    {
        MPU_Get_Gyro(&gyrox,&gyroy,&gyroz);
        MPU_Get_Acc(&accx,&accy,&accz);

        accy=atan2(accx,accz)*180/PI;                 //计算倾角
        gyroBalance = gyroy;
        gyroy=gyroy/16.4;                             //陀螺仪量程转换

        Kalman_Filter(accy,-gyroy);
        return 0;
    }
//	MPU_Get_Gyro(&gyrox,&gyroy,&gyroz);
//	MPU_Get_Acc(&accx,&accy,&accz);
//
//	accy=atan2(accx,accz)*180/PI;                 //计算倾角
//	gyroBalance = gyroy;
//	gyroy=gyroy/16.4;                             //陀螺仪量程转换
//
//	Kalman_Filter(accy,-gyroy);
    angle_control();
    speed_control();
    turn_control();

    leftPwm =  anglePwm + speedPwm + turnPwm;
    rightPwm = anglePwm + speedPwm - turnPwm;

    motor_pwm_set(leftPwm,rightPwm);
//	if(angle>30||angle<-30)
//	{
//		motor_pwm_set(0,0);
//	}
    return 0;
}

void angle_control()
{
    float  angleBias = 0;
    int16_t anglePOut=0,angleDOut=0;
    //angleAim = speedPwm;
    angleBias = angleAim - angle;
    anglePOut = angleBias * angleKp;
    angleDOut = gyroBalance * angleKd;
    anglePwm = anglePOut + angleDOut;

}

void speed_control()
{
    float speedBias = 0,a=0.8;
    int16_t speedPOut=0,speedIOut=0;
    static int16_t speedBiasLowOut=0,speedBiasLowOutLast=0;
    static int16_t speedAdd = 0;

    leftEncode = -read_encoder(2);
    rightEncode = read_encoder(4);
    speedNow = (leftEncode+rightEncode);

    speedBias = speedAim - speedNow;
    speedBiasLowOut = (1-a)*speedBias + a*speedBiasLowOutLast;
    speedBiasLowOutLast = speedBiasLowOut;
    speedAdd+=speedBias;
    if(speedAdd>10000)  speedAdd=10000;             //===积分限幅
    if(speedAdd<-10000)	speedAdd=-10000;              //===积分限幅

    speedPOut = speedBiasLowOut * speedKp;
    speedIOut = speedAdd * speedKi;
    speedPwm = speedPOut + speedIOut;
}

void turn_control()
{
    int16_t turnPOut=0,turnDOut=0;
    turnPOut = 0;
    turnDOut = gyroz * turnKd;
    turnPwm = turnPOut + turnDOut;
}

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) == SET)
    {
        car_control();
    }
    EXTI_ClearITPendingBit(EXTI_Line5);
}

