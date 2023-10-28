#include "bsp_usart.h"
#include "stdio.h"

void DebugUsartMain()
{
    u8 res;
    res = USART_ReceiveData(USART1);
    USART_SendData(USART1,res);
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        DebugUsartMain();
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {

        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

//重定义fputc函数
int fputc(int ch, FILE *f)
{
    while((USART1->SR&0X40)==0);//循环发送,直到发送完毕
    USART1->DR = (u8) ch;
    return ch;
}


void bsp_usart1_init(u32 bound)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|
                           RCC_APB2Periph_GPIOA,  ENABLE);	//使能USART1，GPIOA时钟
    //GPIO端口设置
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        //USART1_TX   GPIOA.9
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //USART1_RX	  GPIOA.10初始化
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    //Usart1 NVIC 配置
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
        NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化NVIC寄存器
    }
    //USART 初始化设置
    {
        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = bound;//串口波特率
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

        USART_Init(USART1, &USART_InitStructure); //初始化串口1
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
        USART_Cmd(USART1, ENABLE);                    //使能串口1
    }
}

void bsp_usart3_init(u32 bound)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,  ENABLE);	//使能USART3，GPIOB时钟
    //GPIO端口设置
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        //USART3_TX   GPIOB.10
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        //USART3_RX	  GPIOB.11初始化
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //Usart3 NVIC 配置
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
        NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化NVIC寄存器
    }
    //USART 初始化设置
    {
        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = bound;//串口波特率
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

        USART_Init(USART3, &USART_InitStructure); //初始化串口1
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
        USART_Cmd(USART3, ENABLE);                    //使能串口1
    }
}


