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

//�ض���fputc����
int fputc(int ch, FILE *f)
{
    while((USART1->SR&0X40)==0);//ѭ������,ֱ���������
    USART1->DR = (u8) ch;
    return ch;
}


void bsp_usart1_init(u32 bound)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|
                           RCC_APB2Periph_GPIOA,  ENABLE);	//ʹ��USART1��GPIOAʱ��
    //GPIO�˿�����
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        //USART1_TX   GPIOA.9
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //USART1_RX	  GPIOA.10��ʼ��
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    //Usart1 NVIC ����
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
        NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��NVIC�Ĵ���
    }
    //USART ��ʼ������
    {
        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

        USART_Init(USART1, &USART_InitStructure); //��ʼ������1
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
        USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1
    }
}

void bsp_usart3_init(u32 bound)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,  ENABLE);	//ʹ��USART3��GPIOBʱ��
    //GPIO�˿�����
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        //USART3_TX   GPIOB.10
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        //USART3_RX	  GPIOB.11��ʼ��
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //Usart3 NVIC ����
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
        NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��NVIC�Ĵ���
    }
    //USART ��ʼ������
    {
        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

        USART_Init(USART3, &USART_InitStructure); //��ʼ������1
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
        USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���1
    }
}


