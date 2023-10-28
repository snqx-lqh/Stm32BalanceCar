#include "stm32_u8g2.h"
#include "bsp_delay.h"


uint8_t u8x8_stm32_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,
                                  U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,
                                  U8X8_UNUSED void *arg_ptr)
{
    switch(msg)
    {
    case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
        __NOP();
        break;
    case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
        for (uint16_t n = 0; n < 320; n++)
        {
            __NOP();
        }
        break;
    case U8X8_MSG_DELAY_MILLI:			                // delay arg_int * 1 milli second
        delay_ms(1);
        break;
    case U8X8_MSG_DELAY_I2C:				            // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
        delay_us(5);
        break;							                // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_SPI_DATA:
        if(arg_int == 1)                                     // arg_int=1: Input dir with pullup high for I2C clock pin
            GPIO_SetBits(GPIOC,GPIO_Pin_14);
        else if(arg_int == 0)
            GPIO_ResetBits(GPIOC,GPIO_Pin_14);
        break;
    case U8X8_MSG_GPIO_SPI_CLOCK:
        if(arg_int == 1)                                     // arg_int=1: Input dir with pullup high for I2C clock pin
            GPIO_SetBits(GPIOC,GPIO_Pin_15);
        else if(arg_int == 0)
            GPIO_ResetBits(GPIOC,GPIO_Pin_15);
        break;
    case U8X8_MSG_GPIO_CS:
        if(arg_int == 1)                                     // arg_int=1: Input dir with pullup high for I2C clock pin
            GPIO_SetBits(GPIOA,GPIO_Pin_8);
        else if(arg_int == 0)
            GPIO_ResetBits(GPIOA,GPIO_Pin_8);
        break;
    case U8X8_MSG_GPIO_DC:
        if(arg_int == 1)                                     // arg_int=1: Input dir with pullup high for I2C clock pin
            GPIO_SetBits(GPIOB,GPIO_Pin_4);
        else if(arg_int == 0)
            GPIO_ResetBits(GPIOB,GPIO_Pin_4);
        break;
    case U8X8_MSG_GPIO_RESET:
        if(arg_int == 1)                                     // arg_int=1: Input dir with pullup high for I2C clock pin
            GPIO_SetBits(GPIOC,GPIO_Pin_13);
        else if(arg_int == 0)
            GPIO_ResetBits(GPIOC,GPIO_Pin_13);
        break;
    case U8X8_MSG_GPIO_MENU_SELECT:
        u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_NEXT:
        u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_PREV:
        u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_HOME:
        u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 1);			         // default return value
        break;
    }
    return 1;
}

//U8g2的初始化，需要调用下面这个u8g2_Setup_ssd1306_128x64_noname_f函数，该函数的4个参数含义：
//u8g2：传入的U8g2结构体
//U8G2_R0：默认使用U8G2_R0即可（用于配置屏幕是否要旋转）
//u8x8_byte_sw_i2c：使用软件IIC驱动，该函数由U8g2源码提供
//u8x8_gpio_and_delay：就是上面我们写的配置函数

void u8g2Init(u8g2_t *u8g2)
{
    u8g2_Setup_ssd1306_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_stm32_gpio_and_delay); // 初始化u8g2 结构体
    u8g2_InitDisplay(u8g2);                                                                       //
    u8g2_SetPowerSave(u8g2, 0);                                                                   //
    u8g2_ClearBuffer(u8g2);
}


void draw(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);

    u8g2_SetFontMode(u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(u8g2, 0); /*字体方向选择*/
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf); /*字库选择*/
    u8g2_DrawStr(u8g2, 0, 20, "U");

    u8g2_SetFontDirection(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_inb16_mf);
    u8g2_DrawStr(u8g2, 21,8,"8");

    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb16_mf);
    u8g2_DrawStr(u8g2, 51,30,"g");
    u8g2_DrawStr(u8g2, 67,30,"\xb2");

	u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_wqy12_t_chinese1);
    u8g2_DrawStr(u8g2, 70,30,"123");
	
    u8g2_SendBuffer(u8g2);
    delay_ms(1000);
}

//画点填充
void testDrawPixelToFillScreen(u8g2_t *u8g2)
{
    int t = 1000;
    u8g2_ClearBuffer(u8g2);

    for (int j = 0; j < 64; j++)
    {
        for (int i = 0; i < 128; i++)
        {
            u8g2_DrawPixel(u8g2,i, j);
        }
    }
    delay_ms(1000);
}