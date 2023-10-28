#ifndef __OLED_H
#define __OLED_H

#include "bsp_sys.h"
#include "stdlib.h"

//-----------------OLED端口定义----------------
#define OLED_RCC_SCL         RCC_APB2Periph_GPIOC
#define OLED_SCL_GPIO        GPIOC
#define OLED_SCL_GPIO_PIN    GPIO_Pin_15

#define OLED_RCC_SDA         RCC_APB2Periph_GPIOC
#define OLED_SDA_GPIO        GPIOC
#define OLED_SDA_GPIO_PIN    GPIO_Pin_14

#define OLED_RCC_RES         RCC_APB2Periph_GPIOC
#define OLED_RES_GPIO        GPIOC
#define OLED_RES_GPIO_PIN    GPIO_Pin_13

#define OLED_RCC_DC         RCC_APB2Periph_GPIOB
#define OLED_DC_GPIO        GPIOB
#define OLED_DC_GPIO_PIN    GPIO_Pin_4

#define OLED_RCC_CS         RCC_APB2Periph_GPIOA
#define OLED_CS_GPIO        GPIOA
#define OLED_CS_GPIO_PIN    GPIO_Pin_8

#define OLED_SCL_Clr() GPIO_ResetBits(OLED_SCL_GPIO,OLED_SCL_GPIO_PIN)//SCL
#define OLED_SCL_Set() GPIO_SetBits(OLED_SCL_GPIO,OLED_SCL_GPIO_PIN)

#define OLED_SDA_Clr() GPIO_ResetBits(OLED_SDA_GPIO,OLED_SDA_GPIO_PIN)//DIN
#define OLED_SDA_Set() GPIO_SetBits(OLED_SDA_GPIO,OLED_SDA_GPIO_PIN)

#define OLED_RES_Clr() GPIO_ResetBits(OLED_RES_GPIO,OLED_RES_GPIO_PIN)//RES
#define OLED_RES_Set() GPIO_SetBits(OLED_RES_GPIO,OLED_RES_GPIO_PIN)

#define OLED_DC_Clr()  GPIO_ResetBits(OLED_DC_GPIO,OLED_DC_GPIO_PIN)//DC
#define OLED_DC_Set()  GPIO_SetBits(OLED_DC_GPIO,OLED_DC_GPIO_PIN)

// #define OLED_CS_Clr()  GPIO_ResetBits(OLED_CS_GPIO,OLED_CS_GPIO_PIN)//CS
// #define OLED_CS_Set()  GPIO_SetBits(OLED_CS_GPIO,OLED_CS_GPIO_PIN)
#define OLED_CS_Clr()
#define OLED_CS_Set()

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

void OLED_ClearPoint(u8 x,u8 y);
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_WR_Byte(u8 dat,u8 mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);
void OLED_DrawCircle(u8 x,u8 y,u8 r);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);
void OLED_ShowChar6x8(u8 x,u8 y,u8 chr,u8 mode);
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode);
void OLED_Init(void);


#endif

