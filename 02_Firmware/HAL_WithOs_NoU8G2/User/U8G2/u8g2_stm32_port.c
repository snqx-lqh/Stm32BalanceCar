/*
������HAL���U8G2�ӿ�
*/


#include "u8g2_stm32_port.h"
#include "i2c.h" 

u8g2_t u8g2;

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buffer[128];
    static uint8_t buf_idx;
    uint8_t *data;
 
    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
    {
        /* add your custom code to init i2c subsystem */
        //I2C��ʼ��
		//HAL�����֮ǰ�ͳ�ʼ�����������ﲻ������
    }
    break;
 
    case U8X8_MSG_BYTE_START_TRANSFER:
    {
        buf_idx = 0;
    }
    break;
 
    case U8X8_MSG_BYTE_SEND:
    {
        data = (uint8_t *)arg_ptr;
 
        while (arg_int > 0)
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
    }
    break;
 
    case U8X8_MSG_BYTE_END_TRANSFER:
    {
		u_i2c_master_transmit_bytes(&hi2c1,u8x8_GetI2CAddress(u8x8), buffer, buf_idx,1000);
    }
    break;
 
    case U8X8_MSG_BYTE_SET_DC:
        break;
 
    default:
        return 0;
    }
 
    return 1;
}
 
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
	switch(msg){
		
//	case U8X8_MSG_GPIO_AND_DELAY_INIT:
//	    break;
		
	case U8X8_MSG_DELAY_MILLI:
		HAL_Delay(arg_int);
	    break;
		
//	case U8X8_MSG_GPIO_I2C_CLOCK:		
//        break;							
//		
//    case U8X8_MSG_GPIO_I2C_DATA:			
//        break;
		
	default:	
		return 0;
	}
	return 1; // command processed successfully.
}
 
//U8g2�ĳ�ʼ������Ҫ�����������u8g2_Setup_ssd1306_128x64_noname_f�������ú�����4���������壺
//u8g2�������U8g2�ṹ��
//U8G2_R0��Ĭ��ʹ��U8G2_R0���ɣ�����������Ļ�Ƿ�Ҫ��ת��
//u8x8_byte_sw_i2c��ʹ�����IIC�������ú�����U8g2Դ���ṩ
//u8x8_gpio_and_delay��������������д�����ú���
 
void u8g2Init(u8g2_t *u8g2)
{
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay); // ��ʼ��u8g2 �ṹ��
	u8g2_InitDisplay(u8g2);                                                                       // 
	u8g2_SetPowerSave(u8g2, 0);                                                                   // 
	u8g2_ClearBuffer(u8g2);
}
 
 
 