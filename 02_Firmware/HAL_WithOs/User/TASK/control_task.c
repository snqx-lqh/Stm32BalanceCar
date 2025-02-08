/**
  ******************************************************************************
  * @file    control_task.c
  * @author  ����Ǳ��(snqx-lgh)
  * @version V
  * @date    2025-1-28
  * @brief   �ⲿ����С��ֱ������Ҫ���񣬿��Ʒ�ʽ��MPU6050�жϴ����󣬻ỽ������
			 ������ִ�нǶȿ��ơ��ٶȿ��ƺ�ת����ƣ����������Ŀ��Ҫ��ѧϰFreeRTOS,
			 ���Ե���ֱ���������Ĳ��Ǻܺã�����ȥ�ұ��˵Ľ̵̳���PID��������д�Ĵ���PID���
			 �ټ�һ�����ٶȻ��о����һЩ��������û����
			 pid�ĵ������ǿ���Bվup _WNNN :https://www.bilibili.com/video/BV1zo4y1D7bx
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

#define  KEY_Q_NUM                5  //���ö��еĳ���
QueueHandle_t KEY_Message_Queue; //�������ݻ������

void control_task(void const * argument)
{
	float accyAngle = 0;    //���ٶȼ�������ĽǶ�
	float gyroyReal = 0;    //���ٶ�ת��������ʵ��ֵ
	uint8_t keyValue = 0;   //�������µ�ֵ
	int16_t rc_lx_value = 0,rc_ry_value = 0;
	//��ȡ��ǰ�����������
    INS_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));
	//��ʼ��һ����������
	KEY_Message_Queue = xQueueCreate(KEY_Q_NUM,1); //�����������ݶ���
	//��ʼ��һЩ����
	balance_car.control_mode = 0;     //ģʽΪ0�ǲ���  1�Ǵ��� �������û����
	balance_car.angleAim = 2.5   ;    //Ŀ��Ƕȣ�Ҳ����ƽ��Ƕ�
	balance_car.angleKp  = -200  ;//
	balance_car.angleKd  = -0.4  ;//
	balance_car.speedAim = 0;
	if(0 == balance_car.control_mode)//ģʽΪ0�ǲ���
	{
		balance_car.speedKp  = -50;  //-140  -70
		balance_car.speedKi  = balance_car.speedKp/200;//-0.7  -0.35
	}else if(1 == balance_car.control_mode)//ģʽΪ1�Ǵ���
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
		//�ȴ��ж��е�����֪ͨ
		while (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) != pdPASS)
        {
        }
		//���6050ԭʼ����
		mpu6050_get_gyro(&balance_car.gyro[0],&balance_car.gyro[1],&balance_car.gyro[2]);
		mpu6050_get_acc(&balance_car.acc[0],&balance_car.acc[1],&balance_car.acc[2]);
		//����Ƕ�
		accyAngle=atan2(balance_car.acc[0],balance_car.acc[2])*180/PI; //���ٶȼ������	
		balance_car.gyroBalance = balance_car.gyro[1];
		gyroyReal=balance_car.gyro[1]/16.4;                            //����������ת��	
		Kalman_getAngle(&KalmanY,accyAngle,-gyroyReal,0.01);           //�������˲���Ƕ�
		balance_car.car_angle = KalmanY.angle;
		
		//��ȡ����ֵ
		keyValue = KeyScan(SINGLE_SCAN);
		if(keyValue != 0)
		{
			xQueueSend(KEY_Message_Queue,&keyValue,0);
		}
		
		//ң�����������
		balance_car.rc_data = get_rc_data();
		//��4�����ݲ�ͬʱΪ2048ʱ����Ϊ��������
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
		
		//�����������PWM
		angle_control(&balance_car);
		speed_control(&balance_car);
		turn_control(&balance_car);
		
		//���ҵ����ֵ
		if(0 == balance_car.control_mode)//ģʽΪ0�ǲ���
		{
			balance_car.leftPwm  = balance_car.anglePwm  + balance_car.speedPwm - balance_car.turnPwm;
			balance_car.rightPwm = balance_car.anglePwm  + balance_car.speedPwm + balance_car.turnPwm;
		}else if(1 == balance_car.control_mode)//ģʽΪ1�Ǵ���
		{
			balance_car.leftPwm  = balance_car.anglePwm  - balance_car.turnPwm;
			balance_car.rightPwm = balance_car.anglePwm  + balance_car.turnPwm;
		}		
		//�����������
		set_dead_time_pwm(&balance_car);
		//�Ƕȳ���һ���Ƕȣ������PWM
		if(balance_car.car_angle > 40 || balance_car.car_angle < -40)
		{
			balance_car.leftPwm  = 0;
			balance_car.rightPwm = 0;
		}
		//����ʵ�ʵ�ֵ
		motor_set_pwm(balance_car.leftPwm,balance_car.rightPwm);
	}
}

/**
  * @brief   ���ƽ�⳵��ز���
  * @param    
  * @retval  balance_car  ƽ�⳵���ƽṹ��
 **/
const balance_car_t *get_car_statu()
{
	return &balance_car;
}

/**
  * @brief   ����ƽ�⳵�ٶ�
  * @param   speed ƽ�⳵�ٶ� 
  * @retval  void
 **/
void set_car_speed(int16_t speed)
{
	balance_car.speedAim = speed;
}

/**
  * @brief   ����ƽ�⳵ת��
  * @param   turn  ƽ�⳵ת��̶� 
  * @retval  void
 **/
void set_car_turn(int16_t turn)
{
	balance_car.turnAim = turn;
}

/**
  * @brief   ���ƽ�⳵�Ƕ�
  * @param    
  * @retval  void
 **/
float get_car_angle()
{
	return balance_car.car_angle;
}

/**
  * @brief   ����ƽ�⳵PID
  * @param   angleKp angleKd speedKp ƽ�⳵PID��ز���
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
  * @brief   ��һ����������
  * @param   balance_car ƽ�⳵���ƽṹ�� 
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
  * @brief   �Ƕȿ�����أ����������ǶȻ���Ҫ��PWM
  * @param   balance_car ƽ�⳵���ƽṹ�� 
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
	if(0 == balance_car->control_mode)//ģʽΪ0�ǲ���
	{
		angleBias = balance_car->angleAim - balance_car->car_angle;
	}else if(1 == balance_car->control_mode)//ģʽΪ1�Ǵ���
	{
		angleBias = balance_car->speedPwm + balance_car->angleAim - balance_car->car_angle;
	}	 
	anglePOut = angleBias   * balance_car->angleKp;
	angleDOut = balance_car->gyroBalance * balance_car->angleKd;
	
	balance_car->anglePwm  = anglePOut + angleDOut;	
}

/**
  * @brief   �ٶȿ�����أ����������ٶȻ���Ҫ��PWM
  * @param   balance_car ƽ�⳵���ƽṹ�� 
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
	//��ȡ������ֵ��Ϊ�ٶ�
	balance_car->leftEncode  = -read_encoder(2);
	balance_car->rightEncode = -read_encoder(4);
	balance_car->speedNow    = (balance_car->leftEncode + balance_car->rightEncode);
	//�����ٶ���Ȼ����һ���˲�
	speedBias   = balance_car->speedAim - balance_car->speedNow;
	speedBiasLowOut     = (1-a)*speedBias + a*speedBiasLowOutLast;
	speedBiasLowOutLast = speedBiasLowOut;
	//��һ��������
	speedAdd    += speedBias;
	if(speedAdd >  10000)   speedAdd=10000;             //===�����޷�
	if(speedAdd < -10000)	speedAdd=-10000;              //===�����޷�	
	//���С�����£��������
	if(balance_car->car_angle > 40 || balance_car->car_angle < -40)
	{
		speedAdd = 0;
	}
	speedPOut = speedBiasLowOut * balance_car->speedKp;
	speedIOut = speedAdd        * balance_car->speedKi;
	balance_car->speedPwm = speedPOut + speedIOut;
}
/**
  * @brief   ת�������أ���������ת����Ҫ��PWM
  * @param   balance_car ƽ�⳵���ƽṹ�� 
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
  * @brief   �жϴ���
  * @param    
  * @retval  void
 **/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  
{   
    if(GPIO_Pin == MPU_INT_Pin)  
    {  
		//����֪ͨ���� 
		if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        {
            static BaseType_t xHigherPriorityTaskWoken;
            vTaskNotifyGiveFromISR(INS_task_local_handler, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }  
}

