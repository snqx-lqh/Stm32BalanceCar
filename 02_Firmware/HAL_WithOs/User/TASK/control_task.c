#include "control_task.h"

#include "math.h"

#include "mpu6050.h"
#include "filter.h"
#include "bsp_motor.h"

static void angle_control(balance_car_t *balance_car);
static void speed_control(balance_car_t *balance_car);
static void turn_control(balance_car_t *balance_car);

balance_car_t balance_car;

static TaskHandle_t INS_task_local_handler;

void control_task(void const * argument)
{
	float accyAngle = 0;    //加速度计算出来的角度
	float gyroyReal = 0;    //角速度转换出来的实际值
	
	//获取当前任务的任务句柄
    INS_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));
	
	//初始化一些变量
	balance_car.angleAim = -2;
	balance_car.angleKp  = -260;
	balance_car.angleKd  = -1.2;
	balance_car.speedAim = 0;
	balance_car.speedKp  = -140;
	balance_car.speedKi  = -0.7;
	balance_car.turnAim = 0;
	balance_car.turnKp  = 0;
	balance_car.turnKd  = 0;
	
	mpu6050_init();
	motor_init();
	
	while(1)
	{
		//等待中断中的任务通知
		while (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) != pdPASS)
        {
        }
		//获得6050原始数据
		mpu6050_get_gyro(&balance_car.gyro[0],&balance_car.gyro[1],&balance_car.gyro[2]);
		mpu6050_get_acc(&balance_car.acc[0],&balance_car.acc[1],&balance_car.acc[2]);
		//计算角度
		accyAngle=atan2(balance_car.acc[0],balance_car.acc[2])*180/PI; //加速度计算倾角	
		balance_car.gyroBalance = balance_car.gyro[1];
		gyroyReal=balance_car.gyro[1]/16.4;                            //陀螺仪量程转换	
		Kalman_getAngle(&KalmanY,accyAngle,-gyroyReal,0.01);           //卡尔曼滤波算角度
		balance_car.car_angle = KalmanY.angle;
		//计算各个环的PWM
		angle_control(&balance_car);
		speed_control(&balance_car);
		turn_control(&balance_car);
		//左右电机赋值
		balance_car.leftPwm  = balance_car.anglePwm + balance_car.speedPwm + balance_car.turnPwm;
		balance_car.rightPwm = balance_car.anglePwm + balance_car.speedPwm - balance_car.turnPwm;
		//设置实际的值
		motor_set_pwm(balance_car.leftPwm,balance_car.rightPwm);
	}
}

/**
  * @brief   获得平衡车相关参数
  * @param    
  * @retval  balance_car  平衡车控制结构体
 **/
const balance_car_t *get_car_statu()
{
	return &balance_car;
}

/**
  * @brief   设置平衡车速度
  * @param   speed 平衡车速度 
  * @retval  void
 **/
void set_car_speed(int16_t speed)
{
	balance_car.speedAim = speed;
}

/**
  * @brief   角度控制相关，最后会计算出角度环需要的PWM
  * @param   balance_car 平衡车控制结构体 
  * @retval  void
 **/
static void angle_control(balance_car_t *balance_car)
{
	float  angleBias = 0;
	int16_t anglePOut=0,angleDOut=0;
	if(balance_car == NULL)
	{
		return;
	}
	angleBias = balance_car->angleAim - balance_car->car_angle;
	anglePOut = angleBias   * balance_car->angleKp;
	angleDOut = balance_car->gyroBalance * balance_car->angleKd;
	
	balance_car->anglePwm  = anglePOut + angleDOut;	
}

/**
  * @brief   速度控制相关，最后会计算出速度环需要的PWM
  * @param   balance_car 平衡车控制结构体 
  * @retval  void
 **/
static void speed_control(balance_car_t *balance_car)
{
	float speedBias = 0,a=0.8;
	int16_t speedPOut=0,speedIOut=0;
	static int16_t speedBiasLowOut=0,speedBiasLowOutLast=0;
	static int16_t speedAdd = 0;
	
	if(balance_car == NULL)
	{
		return;
	}
	//读取编码器值作为速度
	balance_car->leftEncode  = -read_encoder(2);
	balance_car->rightEncode = -read_encoder(4);
	balance_car->speedNow    = (balance_car->leftEncode + balance_car->rightEncode);
	//计算速度误差，然后做一个滤波
	speedBias   = balance_car->speedAim - balance_car->speedNow;
	speedBiasLowOut     = (1-a)*speedBias + a*speedBiasLowOutLast;
	speedBiasLowOutLast = speedBiasLowOut;
	//做一个误差积分
	speedAdd    += speedBias;
	if(speedAdd>10000)  speedAdd=10000;             //===积分限幅
	if(speedAdd<-10000)	speedAdd=-10000;              //===积分限幅	
	
	speedPOut = speedBiasLowOut * balance_car->speedKp;
	speedIOut = speedAdd        * balance_car->speedKi;
	balance_car->speedPwm = speedPOut + speedIOut;
}
/**
  * @brief   转向控制相关，最后会计算出转向环需要的PWM
  * @param   balance_car 平衡车控制结构体 
  * @retval  void
 **/
static void turn_control(balance_car_t *balance_car)
{
	int16_t turnPOut=0,turnDOut=0;
	if(balance_car == NULL)
	{
		return;
	}
	turnPOut = balance_car->turnAim * balance_car->turnKp;
	turnDOut = 0;
	balance_car->turnPwm = turnPOut + turnDOut;
}
/**
  * @brief   中断处理
  * @param    
  * @retval  void
 **/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  
{   
    if(GPIO_Pin == MPU_INT_Pin)  
    {  
		//任务通知唤醒 
		if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        {
            static BaseType_t xHigherPriorityTaskWoken;
            vTaskNotifyGiveFromISR(INS_task_local_handler, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }  
}

