#include "control_task.h"

#include "math.h"

#include "mpu6050.h"
#include "filter.h"
#include "bsp_motor.h"
#include "bsp_key.h" 

static void angle_control(balance_car_t *balance_car);
static void speed_control(balance_car_t *balance_car);
static void turn_control(balance_car_t *balance_car);
static void set_dead_time_pwm(balance_car_t *balance_car);

balance_car_t balance_car;

static TaskHandle_t INS_task_local_handler;

void control_task(void const * argument)
{
	float accyAngle = 0;    //���ٶȼ�������ĽǶ�
	float gyroyReal = 0;    //���ٶ�ת��������ʵ��ֵ
	
	//��ȡ��ǰ�����������
    INS_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));
	
	//��ʼ��һЩ����
	balance_car.control_mode = 1; //ģʽΪ0�ǲ���  1�Ǵ���
	balance_car.angleAim = -6.2;
	balance_car.angleKp  = -350 ;//-300
	balance_car.angleKd  = -1 ;//-0.5
	balance_car.angleKp  *= 0.6; 
	balance_car.angleKd  *= 0.6; 
	balance_car.speedAim = 0;
	if(0 == balance_car.control_mode)//ģʽΪ0�ǲ���
	{
		balance_car.speedKp  = -70;  //-140  -70
		balance_car.speedKi  = balance_car.speedKp/200;//-0.7  -0.35
	}else if(1 == balance_car.control_mode)//ģʽΪ1�Ǵ���
	{
		balance_car.speedKp  = 0.32;
		balance_car.speedKi  = balance_car.speedKp/200;	
	}
	balance_car.turnAim = 0;
	balance_car.turnKp  = 0;
	balance_car.turnKd  = 0.1;
	
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
		//�����������PWM
		angle_control(&balance_car);
		speed_control(&balance_car);
		turn_control(&balance_car);
		//���ҵ����ֵ
		if(0 == balance_car.control_mode)//ģʽΪ0�ǲ���
		{
			balance_car.leftPwm  = balance_car.anglePwm  + balance_car.speedPwm + balance_car.turnPwm;
			balance_car.rightPwm = balance_car.anglePwm  + balance_car.speedPwm - balance_car.turnPwm;
		}else if(1 == balance_car.control_mode)//ģʽΪ1�Ǵ���
		{
			balance_car.leftPwm  = balance_car.anglePwm    + balance_car.turnPwm;
			balance_car.rightPwm = balance_car.anglePwm    - balance_car.turnPwm;
		}		
		//�����������
		set_dead_time_pwm(&balance_car);
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
	
	if(balance_car->leftPwm > 0) balance_car->leftPwm += 300;
	else balance_car->leftPwm -= 300;
	
	if(balance_car->rightPwm > 0) balance_car->rightPwm += 300;
	else balance_car->rightPwm -= 300;
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
	if(speedAdd>10000)  speedAdd=10000;             //===�����޷�
	if(speedAdd<-10000)	speedAdd=-10000;              //===�����޷�	
	
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

