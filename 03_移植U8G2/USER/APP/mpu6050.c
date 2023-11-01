#include "mpu6050.h"

#include "bsp_delay.h"

int16_t gyrox=0,gyroy=0,gyroz=0;
int16_t accx =0, accy=0, accz=0;

//MPU IIC 延时函数
void MPU_IIC_Delay(void)
{
    delay_us(2);
}

void MPU_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //开启端口时钟
    RCC_APB2PeriphClockCmd(IIC_SCL_PORT_RCC | IIC_SDA_PORT_RCC, ENABLE);

    //配置SCL、SDA引脚
    GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //推挽输出
    GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //推挽输出
    GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);

    IIC_SCL = 1;
    IIC_SDA = 1; //所有设备空闲，总线拉高电平
}

//切换SDA引脚工作模式为推挽输出
void SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //推挽输出
    GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
//    IIC_SDA_PORT->CRL&=0X0FFFFFFF;
//    IIC_SDA_PORT->CRL|=3<<28;
}

//切换SDA引脚工作模式为上拉输入
void SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
    GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
//    IIC_SDA_PORT->CRL&=0X0FFFFFFF;
//    IIC_SDA_PORT->CRL|=8<<28;
}

//产生起始信号
void IIC_Start(void)
{
    SDA_OUT(); // SDA线输出模式

    IIC_SDA = 1;
    IIC_SCL = 1;
    MPU_IIC_Delay();
    IIC_SDA = 0; // START:when CLK is high,DATA change form high to low
    MPU_IIC_Delay();
    IIC_SCL = 0; //钳住I2C总线，准备发送或接收数据
}

//产生停止信号
void IIC_Stop(void)
{
    SDA_OUT(); // SDA线输出模式
    IIC_SCL = 0;
    IIC_SDA = 0;
    MPU_IIC_Delay();
    IIC_SCL = 1;
    IIC_SDA = 1; // STOP:when CLK is high,DATA change form low to high
    MPU_IIC_Delay();
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime=0;
    SDA_IN();      //SDA设置为输入
    IIC_SDA=1;
    MPU_IIC_Delay();
    IIC_SCL=1;
    MPU_IIC_Delay();
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL=0;//时钟输出0
    return 0;
}

//产生应答信号
void IIC_Ack(void)
{
    IIC_SCL = 0;
    SDA_OUT(); // SDA设置为输出模式
    IIC_SDA = 0; //发送低电平
    MPU_IIC_Delay();
    IIC_SCL = 1;
    MPU_IIC_Delay();
    IIC_SCL = 0;
}

//不产生应答
void IIC_NAck(void)
{
    IIC_SCL = 0;
    SDA_OUT(); // SDA设置为输出模式
    IIC_SDA = 1; //发送高电平
    MPU_IIC_Delay();
    IIC_SCL = 1;
    MPU_IIC_Delay();
    IIC_SCL = 0;
}

//发送一个字节（IIC发送数据以字节为单位，从高位开始发送）
void IIC_Send_Byte(u8 txd)
{
    u8 i;
    SDA_OUT();   // SDA设置为输出模式
    IIC_SCL = 0; //拉低时钟开始数据传输
    for (i = 0; i < 8; i++)
    {
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        IIC_SCL=1;
        MPU_IIC_Delay();
        IIC_SCL=0;
        MPU_IIC_Delay();
    }
}

//读取一个字节，参数传入1为应答，0为非应答
u8 IIC_Read_Byte(u8 ack)
{
    u8 i,receive = 0; //存放读取的字节

    SDA_IN(); // SDA设置为输入模式

    for (i = 0; i < 8; i++)
    {
        IIC_SCL = 0;
        MPU_IIC_Delay();
        IIC_SCL = 1;
        receive <<= 1; //左移，最低位填充0
        if (READ_SDA)
        {
            receive++; //如果收到1，最低位置1
        }
        MPU_IIC_Delay();
    }

    if (!ack)
        IIC_NAck(); //非应答
    else
        IIC_Ack(); //应答

    return receive;
}

//从指定位置读一个字节
u8 MPU_Read_Byte(u8 addr,u8 reg)
{
    u8 receive = 0;

    IIC_Start();
    IIC_Send_Byte((addr<<1)|0);     //总线寻址：向总线发送IIC设备地址0XA0,读写方向为0写数据
    IIC_Wait_Ack();          //等待应答，建立通信连接
    IIC_Send_Byte(reg); //向设备发送要操作的地址
    IIC_Wait_Ack();
    IIC_Start();         //读写方向改变时，需要重新发送起始信号
    IIC_Send_Byte((addr<<1)|1); //读写方向为1读数据
    IIC_Wait_Ack();
    receive = IIC_Read_Byte(0); //接收一个字节，发送非应答信号
    IIC_Stop();

    return receive;
}

//向指定位置写一个字节
u8 MPU_Write_Byte(u8 addr,u8 reg,u8 data)
{
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0);      //总线寻址：向总线发送IIC设备地址0XA0,读写方向为0写数据
    if(IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg); //向设备发送要操作的地址
    IIC_Wait_Ack();
    IIC_Send_Byte(data); //发送一个字节
    if(IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Stop();
    return 0;
}

//从指定位置读NumToRead个字节
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //发送器件地址+写命令
    if(IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
    IIC_Start();
    IIC_Send_Byte((addr<<1)|1); //发送器件地址+读命令
    IIC_Wait_Ack();             //等待应答
    while(len)
    {
        if(len==1)*buf=IIC_Read_Byte(0);//读数据,发送nACK
        else *buf=IIC_Read_Byte(1);		//读数据,发送ACK
        len--;
        buf++;
    }
    IIC_Stop();                 //产生一个停止条件
    return 0;
}

//向指定位置写NumToWrite个字节
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    u8 i;
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //发送器件地址+写命令
    if(IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
    for(i=0; i<len; i++)
    {
        IIC_Send_Byte(buf[i]);  //发送数据
        if(IIC_Wait_Ack())      //等待ACK
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_Stop();
    return 0;
}

u8 res=0;
//初始化
u8 MPU_Init(void)
{
    MPU_IIC_Init();     //初始化IIC总线
    MPU_Write_Byte(MPU6050_ADDR,MPU_PWR_MGMT1_REG,0X80);//复位MPU9250
    delay_ms(100);  //延时100ms
    MPU_Write_Byte(MPU6050_ADDR,MPU_PWR_MGMT1_REG,0X00);//唤醒MPU9250
    MPU_Set_Gyro_Fsr(3);					        	//陀螺仪传感器,±2000dps
    MPU_Set_Accel_Fsr(0);					       	 	//加速度传感器,±2g
    MPU_Set_Rate(200);						       	 	//设置采样率200Hz
    MPU_Write_Byte(MPU6050_ADDR,MPU_INT_EN_REG,0X01);   //数据中断
    MPU_Write_Byte(MPU6050_ADDR,MPU_USER_CTRL_REG,0X00);//I2C主模式关闭
    MPU_Write_Byte(MPU6050_ADDR,MPU_FIFO_EN_REG,0X00);	//关闭FIFO
    MPU_Write_Byte(MPU6050_ADDR,MPU_INTBP_CFG_REG,0X9C);//选择中断电平，逻辑电平为0  50us高   查询立即自动清除
    res=MPU_Read_Byte(MPU6050_ADDR,MPU_DEVICE_ID_REG);  //读取MPU6500的ID
    if(res==0X68) //器件ID正确
    {
        MPU_Write_Byte(MPU6050_ADDR,MPU_PWR_MGMT1_REG,0X01);  	//设置CLKSEL,PLL X轴为参考
        MPU_Write_Byte(MPU6050_ADDR,MPU_PWR_MGMT2_REG,0X00);  	//加速度与陀螺仪都工作
        MPU_Set_Rate(200);						       	//设置采样率为200Hz
    } else return 1;
    return 0;
}

//设置陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU6050_ADDR,MPU_GYRO_CFG_REG,(fsr<<3)|3);//设置陀螺仪满量程范围
}
//设置加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU6050_ADDR,MPU_ACCEL_CFG_REG,fsr<<3);//设置加速度传感器满量程范围
}

//设置数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败
u8 MPU_Set_LPF(u16 lpf)
{
    u8 data=0;
    if(lpf>=188)data=1;
    else if(lpf>=98)data=2;
    else if(lpf>=42)data=3;
    else if(lpf>=20)data=4;
    else if(lpf>=10)data=5;
    else data=6;
    return MPU_Write_Byte(MPU6050_ADDR,MPU_CFG_REG,data);//设置数字低通滤波器
}

//设置MPU9250的采样率(假定Fs=1KHz)
//rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败
u8 MPU_Set_Rate(u16 rate)
{
    u8 data;
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    data=1000/rate-1;
    data=MPU_Write_Byte(MPU6050_ADDR,MPU_SAMPLE_RATE_REG,data);	//设置数字低通滤波器
    return MPU_Set_LPF(rate/2);	//自动设置LPF为采样率的一半
}

//得到温度值
//返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
    u8 buf[2];
    short raw;
    float temp;
    MPU_Read_Len(MPU6050_ADDR,MPU_TEMP_OUTH_REG,2,buf);
    raw=((u16)buf[0]<<8)|buf[1];
    temp=21+((double)raw)/333.87;
    return temp*100;;
}
///////////////////////////////////////////////////
/////////////////////////////////
//得到陀螺仪值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
u8 MPU_Get_Gyro(short *gx,short *gy,short *gz)
{
    u8 buf[6],res;
    res=MPU_Read_Len(MPU6050_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
    if(res==0)
    {
        *gx=(((u16)buf[0]<<8)|buf[1]);
        *gy=(((u16)buf[2]<<8)|buf[3]);
        *gz=(((u16)buf[4]<<8)|buf[5]);
    }
    return res;
}

//得到加速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
u8 MPU_Get_Acc(short *ax,short *ay,short *az)
{
    u8 buf[6],res;
    res=MPU_Read_Len(MPU6050_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
    if(res==0)
    {
        *ax=(((u16)buf[0]<<8)|buf[1]);
        *ay=(((u16)buf[2]<<8)|buf[3]);
        *az=(((u16)buf[4]<<8)|buf[5]);
    }
    return res;
}

