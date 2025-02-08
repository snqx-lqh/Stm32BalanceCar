#include "mpu6050.h"

/********************User modification area begin********************/
//ֻ��Ҫ���Լ���IIC��������滻��ȥ����
#include "main.h"
#include "i2c.h"

void mpu6050_delay_ms(uint16_t xms)
{
	HAL_Delay(xms);
}

int mpu6050_write_one_byte(unsigned char reg,unsigned char data)
{
	u_i2c1_write_byte(MPU6050_ADDR,reg,&data);
	return 0;
}

int mpu6050_read_one_byte(unsigned char reg,unsigned char *data)
{
	u_i2c1_read_byte(MPU6050_ADDR,reg,data);
	return 0;
}

int mpu6050_read_bytes(unsigned char reg,unsigned char *data,unsigned char len)
{
	u_i2c1_read_bytes(MPU6050_ADDR,reg,data,len);
	return 0;
}

/*********************User modification area end**************/

int mpu6050_init()
{
	uint8_t res = 0;
	mpu6050_write_one_byte(MPU_PWR_MGMT1_REG,0X80); 
    mpu6050_delay_ms(100);  
    mpu6050_write_one_byte(MPU_PWR_MGMT1_REG,0X00); 
    mpu6050_set_gyro_fsr(GYRO_2000DPS);					        	
    mpu6050_set_acc_fsr(ACC_2G);					       	 	
    mpu6050_set_rate(100);						       	 	
    mpu6050_write_one_byte(MPU_INT_EN_REG,0X01);    
    mpu6050_write_one_byte(MPU_USER_CTRL_REG,0X00); 
    mpu6050_write_one_byte(MPU_FIFO_EN_REG,0X00);	  
    mpu6050_write_one_byte(MPU_INTBP_CFG_REG,0X9C); 
    mpu6050_read_one_byte (MPU_DEVICE_ID_REG,&res); 
    if(res==MPU6050_ADDR)  
    {
        mpu6050_write_one_byte(MPU_PWR_MGMT1_REG,0X01);  	 
        mpu6050_write_one_byte(MPU_PWR_MGMT2_REG,0X00);  	 
        mpu6050_set_rate(100);						       	 
    } else return 1;
    return 0;
}

//���������Ǵ����������̷�Χ
uint8_t mpu6050_set_gyro_fsr(uint8_t fsr)
{
    return mpu6050_write_one_byte(MPU_GYRO_CFG_REG,(fsr<<3)|3);//���������������̷�Χ
}

//���ü��ٶȴ����������̷�Χ
uint8_t mpu6050_set_acc_fsr(uint8_t fsr)
{
    return mpu6050_write_one_byte(MPU_ACCEL_CFG_REG,fsr<<3);//���ü��ٶȴ����������̷�Χ
}

//�������ֵ�ͨ�˲���
uint8_t mpu6050_set_lpf(uint16_t lpf)
{
    uint8_t data=0;
    if(lpf>=188)data=1;
    else if(lpf>=98)data=2;
    else if(lpf>=42)data=3;
    else if(lpf>=20)data=4;
    else if(lpf>=10)data=5;
    else data=6;
    return mpu6050_write_one_byte(MPU_CFG_REG,data);//�������ֵ�ͨ�˲���
}

//����MPU6050�Ĳ�����
uint8_t mpu6050_set_rate(uint16_t rate)
{
    uint8_t data;
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    data=1000/rate-1;
    data=mpu6050_write_one_byte(MPU_SAMPLE_RATE_REG,data);	//�������ֵ�ͨ�˲���
    return mpu6050_set_lpf(rate/2);	//�Զ�����LPFΪ�����ʵ�һ��
}
/**
  * @brief   ����������¶�ֵ
  * @param    
  * @retval  �������¶�
 **/
short mpu6050_get_temperature(void)
{
    uint8_t buf[2];
    short raw;
    mpu6050_read_bytes(MPU_TEMP_OUTH_REG,buf,2);
    raw=((uint16_t)buf[0]<<8)|buf[1];
    return raw;
}
/**
  * @brief   ��������ǽ��ٶ�ֵ
  * @param   �����ȡ���ٶȵĵ�ַ
  * @retval  0 ��ȷ��ȡ 1 IIC��ȡʧ�� 2 ����ָ��Ϊ�� 
 **/
uint8_t mpu6050_get_gyro(int16_t *gx,int16_t *gy,int16_t *gz)
{
    uint8_t buf[6],res;
	if(gx == NULL || gy == NULL || gz == NULL)
		return 2;
    res=mpu6050_read_bytes(MPU_GYRO_XOUTH_REG,buf,6);
    if(res==0)
    {
        *gx=(((uint16_t)buf[0]<<8)|buf[1]);
        *gy=(((uint16_t)buf[2]<<8)|buf[3]);
        *gz=(((uint16_t)buf[4]<<8)|buf[5]);
    }
    return res;
}
/**
  * @brief   ��������Ǽ��ٶȼ�ֵ
  * @param   �����ȡ���ٶȵĵ�ַ
  * @retval  0 ��ȷ��ȡ 1 IIC��ȡʧ�� 2 ����ָ��Ϊ�� 
 **/
uint8_t mpu6050_get_acc(int16_t *ax,int16_t *ay,int16_t *az)
{
    uint8_t buf[6],res;
	if(ax == NULL || ay == NULL || az == NULL)
		return 2;
    res=mpu6050_read_bytes(MPU_ACCEL_XOUTH_REG,buf,6);
    if(res==0)
    {
        *ax=(((uint16_t)buf[0]<<8)|buf[1]);
        *ay=(((uint16_t)buf[2]<<8)|buf[3]);
        *az=(((uint16_t)buf[4]<<8)|buf[5]);
    }
    return res;
}

