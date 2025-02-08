/**
  ******************************************************************************
  * @file    control_task.c
  * @author  少年潜行(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   这部分是小车直立的主要任务，控制方式是MPU6050中断触发后，会唤醒任务，
			 任务中执行角度控制、速度控制和转向控制，由于这个项目主要是学习FreeRTOS,
			 所以调试直立部分做的不是很好，可以去找别人的教程调试PID，而且我写的串级PID如果
			 再加一个角速度环感觉会好一些，但是我没做。
			 pid的调试我是看的B站up _WNNN :https://www.bilibili.com/video/BV1zo4y1D7bx
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; Copyright {Year} LQH,China</center></h2>
  ******************************************************************************
  */
#include "control_task.h"

#include "math.h"

#include "mpu6050.h"
#include "filter.h"
#include "bsp_motor.h"
#include "bsp_key.h" 

#include "queue.h"

#define RC_CONTROL 0

static void angle_control(balance_car_t *balance_car);
static void speed_control(balance_car_t *balance_car);
static void turn_control(balance_car_t *balance_car);
static void set_dead_time_pwm(balance_car_t *balance_car);

balance_car_t balance_car;

static TaskHandle_t INS_task_local_handler;

#define  KEY_Q_NUM                5  //设置队列的长度
QueueHandle_t KEY_Message_Queue; //按键数据缓冲队列

void control_task(void const * argument)
{
	float accyAngle = 0;    //加速度计算出来的角度
	float gyroyReal = 0;    //角速度转换出来的实际值
	uint8_t keyValue = 0;   //按键按下的值
	int16_t rc_lx_value = 0,rc_ry_value = 0;
	//获取当前任务的任务句柄
    INS_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));
	//初始化一个按键队列
	KEY_Message_Queue = xQueueCreate(KEY_Q_NUM,1); //创建蓝牙数据队列
	//初始化一些变量
	balance_car.control_mode = 0;     //模式为0是并级  1是串级 这个参数没调好
	balance_car.angleAim = 2.5   ;    //目标角度，也就是平衡角度
	balance_car.angleKp  = -200  ;//
	balance_car.angleKd  = -0.4  ;//
	balance_car.speedAim = 0;
	if(0 == balance_car.control_mode)//模式为0是并级
	{
		balance_car.speedKp  = -50;  //-140  -70
		balance_car.speedKi  = balance_car.speedKp/200;//-0.7  -0.35
	}else if(1 == balance_car.control_mode)//模式为1是串级
	{
		balance_car.speedKp  = 0.32;
		balance_car.speedKi  = balance_car.speedKp/200;	
	}
	balance_car.turnAim = 0;
	balance_car.turnKp  = 10;
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
		
		//获取按键值
		keyValue = KeyScan(SINGLE_SCAN);
		if(keyValue != 0)
		{
			xQueueSend(KEY_Message_Queue,&keyValue,0);
		}
		
		//遥控器控制相关
		balance_car.rc_data = get_rc_data();
		//当4个数据不同时为2048时，认为有数据来
		if(balance_car.rc_data->lx_value != 2048 && 
		   balance_car.rc_data->ly_value != 2048 &&
		   balance_car.rc_data->rx_value != 2048 &&
		   balance_car.rc_data->ry_value != 2048)
		{
			rc_lx_value = (int16_t)((balance_car.rc_data->lx_value - 2048)/64.0f);
			rc_ry_value = (int16_t)((balance_car.rc_data->ry_value - 2048)/64.0f);
			balance_car.speedAim = rc_lx_value;
			balance_car.turnAim  = rc_ry_value;
		}
		
		//计算各个环的PWM
		angle_control(&balance_car);
		speed_control(&balance_car);
		turn_control(&balance_car);
		
		//左右电机赋值
		if(0 == balance_car.control_mode)//模式为0是并级
		{
			balance_car.leftPwm  = balance_car.anglePwm  + balance_car.speedPwm - balance_car.turnPwm;
			balance_car.rightPwm = balance_car.anglePwm  + balance_car.speedPwm + balance_car.turnPwm;
		}else if(1 == balance_car.control_mode)//模式为1是串级
		{
			balance_car.leftPwm  = balance_car.anglePwm  - balance_car.turnPwm;
			balance_car.rightPwm = balance_car.anglePwm  + balance_car.turnPwm;
		}		
		//添加死区控制
		set_dead_time_pwm(&balance_car);
		//角度超过一定角度，就清除PWM
		if(balance_car.car_angle > 40 || balance_car.car_angle < -40)
		{
			balance_car.leftPwm  = 0;
			balance_car.rightPwm = 0;
		}
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
  * @brief   设置平衡车转向
  * @param   turn  平衡车转向程度 
  * @retval  void
 **/
void set_car_turn(int16_t turn)
{
	balance_car.turnAim = turn;
}

/**
  * @brief   获得平衡车角度
  * @param    
  * @retval  void
 **/
float get_car_angle()
{
	return balance_car.car_angle;
}

/**
  * @brief   设置平衡车PID
  * @param   angleKp angleKd speedKp 平衡车PID相关参数
  * @retval  void
 **/
void set_car_pid(float angleKp,float angleKd,float speedKp)
{
	balance_car.angleKp = angleKp;
	balance_car.angleKd = angleKd;
	balance_car.speedKp = speedKp;
	balance_car.speedKi = balance_car.speedKp/200;
}
/**
  * @brief   给一个死区控制
  * @param   balance_car 平衡车控制结构体 
  * @retval  void
 **/
static void set_dead_time_pwm(balance_car_t *balance_car)
{
	if(balance_car == NULL)
	{
		return;
	}
	
	if(balance_car->leftPwm > 0) balance_car->leftPwm += 200;
	else balance_car->leftPwm -= 200;
	
	if(balance_car->rightPwm > 0) balance_car->rightPwm += 200;
	else balance_car->rightPwm -= 200;
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
	if(0 == balance_car->control_mode)//模式为0是并级
	{
		angleBias = balance_car->angleAim - balance_car->car_angle;
	}else if(1 == balance_car->control_mode)//模式为1是串级
	{
		angleBias = balance_car->speedPwm + balance_car->angleAim - balance_car->car_angle;
	}	 
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
	if(speedAdd >  10000)   speedAdd=10000;             //===积分限幅
	if(speedAdd < -10000)	speedAdd=-10000;              //===积分限幅	
	//如果小车倒下，清除积分
	if(balance_car->car_angle > 40 || balance_car->car_angle < -40)
	{
		speedAdd = 0;
	}
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
	turnDOut = balance_car->gyro[2] * balance_car->turnKd;
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

