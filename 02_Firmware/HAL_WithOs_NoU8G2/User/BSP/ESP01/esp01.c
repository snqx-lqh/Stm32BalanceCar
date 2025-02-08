/**
  ******************************************************************************
  * @file    esp01.c 
  * @author  少年潜行(snqx-lqh)
  * @version V1.0
  * @date    2024-10-01
  * @brief   参考安信可代码修改，esp01的相关API
  ******************************************************************************
  * @attention
  *			V1.0 目前只调试了支持AP模式相关的使用，使用串口空闲中断接收数据，但是
  *              感觉这里的处理能优化，但是暂时没想到方法
  *
  * <h2><center>&copy; Copyright 2024 LQH,China</center></h2>
  ******************************************************************************
  */
#include "esp01.h"
#include <stdarg.h>

#define ESP_DEBUG 1

/********************User modification area begin*******************/
#include "usart.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define esp_delay_ms(xms) vTaskDelay(xms)

void esp01_uart_send_data(uint8_t *data,uint16_t data_len)
{
	HAL_UART_Transmit(&huart2, data , data_len,10 );
}

/********************User modification area end  *******************/

struct STRUCT_USART_Fram ESP8266_Fram_Record_Struct = { 0 };  //定义了一个数据帧结构体
//用来作为延时判断，由于使用的是空闲中断接收，想使用一个方法来确认当前
//接收到的东西是不是同一个命令回来的数据
int16_t send_count      = 0;
//判断是使用AT指令的时候，还是普通接收数据时候
int16_t esp_config_mode = 0;

int16_t get_send_count()
{
	return send_count;
}

int16_t get_esp_config_mode()
{
	return esp_config_mode;
}

/**
  * @brief   对ESP8266模块发送AT指令
  * @param   cmd 待发送的指令
  * @param   ack1,ack2;期待的响应，为NULL表不需响应，两者为或逻辑关系
  * @param   time 等待响应时间
  * @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_Send_AT_Cmd(char *cmd,char *ack1,char *ack2,u32 time)
{ 
	uint8_t usart_send[30] = {0};
	uint8_t send_len = 0;
	
	if(cmd == NULL)
		return false;
	
	send_count ++;
	esp_config_mode  = 1;
    ESP8266_Fram_Record_Struct .InfBit .FramLength = 0; //重新接收新的数据包
    sprintf((char*)usart_send,"%s\r\n", cmd);
	send_len = strlen((char*)usart_send);
	esp01_uart_send_data(  usart_send , send_len );
    if(ack1==0 && ack2==0)     //不需要接收数据
    {
		return true;
    }
    esp_delay_ms(time);   //延时
    ESP8266_Fram_Record_Struct.Data_RX_BUF[ESP8266_Fram_Record_Struct.InfBit.FramLength ] = '\0';

#if ESP_DEBUG == 1		
    printf("%s",ESP8266_Fram_Record_Struct .Data_RX_BUF);
#endif
	
	esp_config_mode = 0;
    if(ack1!=0&&ack2!=0)
    {
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack1 ) || 
                 ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack2 ) );
    }
    else if( ack1 != 0 )  //strstr(s1,s2);检测s2是否为s1的一部分，是返回该位置，否则返回false，它强制转换为bool类型了
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack1 ) );

    else
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack2 ) );

}
/**
  * @brief   选择ESP8266的工作模式
  * @param   enumMode 模式类型 STA AP STA_AP
  * @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
    switch ( enumMode )
    {
        case STA:
            return ESP8266_Send_AT_Cmd ( "AT+CWMODE=1", "OK", "no change", 2500 ); 

        case AP:
            return ESP8266_Send_AT_Cmd ( "AT+CWMODE=2", "OK", "no change", 2500 ); 

        case STA_AP:
            return ESP8266_Send_AT_Cmd ( "AT+CWMODE=3", "OK", "no change", 2500 ); 

        default:
          return false;
    }       
}

/**
  * @brief   ESP8266连接外部的WIFI
  * @param   pSSID WiFi帐号   pPassWord WiFi密码
  * @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_JoinAP( char * pSSID, char * pPassWord)
{
    char cCmd [120];
	if(pSSID == NULL || pPassWord == NULL)
		return false;
    sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 5000 );
}

/**
  * @brief   ESP8266设置多连接
  * @param   enumEnUnvarnishTx  是否多连接，bool类型
  * @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_Enable_MultipleId (FunctionalState enumEnUnvarnishTx )
{
    char cStr [20];

    sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );

    return ESP8266_Send_AT_Cmd ( cStr, "OK", 0, 500 );

}

/**
  * @brief   ESP8266创建 TCP 服务器  默认端口为 333
  * @param   
  * @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_Create_Tcp_server (  )
{
    char cStr [20];

    sprintf ( cStr, "AT+CIPSERVER=1" );

    return ESP8266_Send_AT_Cmd ( cStr, "OK", 0, 500 );

}

/**
	* @brief   ESP8266 发送一段数据
	* @param   str 待发送数据 uid 要发送的对象ID strlen 待发送数据长度
	* @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_Send_Str (char* str,uint8_t uid,uint16_t strlen)
{
    char cStr [20];
	
    sprintf ( cStr, "AT+CIPSEND=%d,%d" ,uid,strlen);
	
	if(ESP8266_Send_AT_Cmd ( cStr, ">", 0, 500 ))
	{
		esp01_uart_send_data( (uint8_t*)str,strlen);		 
		return true;
	}
	
    return false;

}

/**
	* @brief   ESP8266 连接服务器
	* @param   enumE  网络类型
	* @param   ip ，服务器IP
	* @param   ComNum  服务器端口
	* @param   id，连接号，确保通信不受外界干扰
	* @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
    char cStr [100] = { 0 }, cCmd [120];

    switch (  enumE )
    {
        case enumTCP:
          sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
          break;

        case enumUDP:
          sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
          break;

        default:
            break;
    }

    if ( id < 5 )
        sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);

    else
        sprintf ( cCmd, "AT+CIPSTART=%s", cStr );

    return ESP8266_Send_AT_Cmd ( cCmd, "OK", "ALREAY CONNECT", 4000 );

}

/**
	* @brief   透传使能
	* @retval  返回 true 发送成功，false 失败
 **/
bool ESP8266_UnvarnishSend ( void )
{
    if (!ESP8266_Send_AT_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 ))
        return false;

    return 
        ESP8266_Send_AT_Cmd( "AT+CIPSEND", "OK", ">", 500 );
}

/**
	* @brief   ESP8266发送字符串
	* @param   enumEnUnvarnishTx是否使能透传模式
	* @param   pStr字符串
	* @param   ulStrLength字符串长度
	* @param   ucId 连接号
	* @retval  返回 true 发送成功，false 失败
 **/ 
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
    char cStr [20];
    bool bRet = false;
	uint8_t usart_send[30] = {0};
	uint8_t send_len = 0;

    if ( enumEnUnvarnishTx )
    {
		sprintf((char*)usart_send,"%s", pStr);
		send_len = strlen((char*)usart_send);
		esp01_uart_send_data(  usart_send , send_len );
        bRet = true;
    }
    else
    {
        if ( ucId < 5 )
            sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );
        else
            sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength + 2 );

        ESP8266_Send_AT_Cmd ( cStr, "> ", 0, 1000 );

        bRet = ESP8266_Send_AT_Cmd ( pStr, "SEND OK", 0, 1000 );
	}

    return bRet;

}

/**
	* @brief   退出透传
	* @retval  返回 true 发送成功，false 失败
 **/
void ESP8266_ExitUnvarnishSend ( void )
{
	uint8_t usart_send[30] = {0};
	uint8_t send_len = 0;
    esp_delay_ms(1000);
	sprintf((char*)usart_send,"+++");
	send_len = strlen((char*)usart_send);
	esp01_uart_send_data(  usart_send , send_len );
    esp_delay_ms( 500 );    
}

/**
	* @brief   ESP8266 检测连接状态
	* @retval  返回0：获取状态失败
	* @retval  返回2：获得ip
	* @retval  返回3：建立连接 
	* @retval  返回4：失去连接 
 **/
uint8_t ESP8266_Get_LinkStatus ( void )
{
    if (ESP8266_Send_AT_Cmd( "AT+CIPSTATUS", "OK", 0, 500 ) )
    {
        if ( strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, "STATUS:2\r\n" ) )
            return 2;

        else if ( strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, "STATUS:3\r\n" ) )
            return 3;

        else if ( strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, "STATUS:4\r\n" ) )
            return 4;       
    }
    return 0;
}




