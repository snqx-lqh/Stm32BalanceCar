/**
  ******************************************************************************
  * @file    esp01.c 
  * @author  ����Ǳ��(snqx-lqh)
  * @version V1.0
  * @date    2024-10-01
  * @brief   �ο����ſɴ����޸ģ�esp01�����API
  ******************************************************************************
  * @attention
  *			V1.0 Ŀǰֻ������֧��APģʽ��ص�ʹ�ã�ʹ�ô��ڿ����жϽ������ݣ�����
  *              �о�����Ĵ������Ż���������ʱû�뵽����
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

struct STRUCT_USART_Fram ESP8266_Fram_Record_Struct = { 0 };  //������һ������֡�ṹ��
//������Ϊ��ʱ�жϣ�����ʹ�õ��ǿ����жϽ��գ���ʹ��һ��������ȷ�ϵ�ǰ
//���յ��Ķ����ǲ���ͬһ���������������
int16_t send_count      = 0;
//�ж���ʹ��ATָ���ʱ�򣬻�����ͨ��������ʱ��
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
  * @brief   ��ESP8266ģ�鷢��ATָ��
  * @param   cmd �����͵�ָ��
  * @param   ack1,ack2;�ڴ�����Ӧ��ΪNULL������Ӧ������Ϊ���߼���ϵ
  * @param   time �ȴ���Ӧʱ��
  * @retval  ���� true ���ͳɹ���false ʧ��
 **/
bool ESP8266_Send_AT_Cmd(char *cmd,char *ack1,char *ack2,u32 time)
{ 
	uint8_t usart_send[30] = {0};
	uint8_t send_len = 0;
	
	if(cmd == NULL)
		return false;
	
	send_count ++;
	esp_config_mode  = 1;
    ESP8266_Fram_Record_Struct .InfBit .FramLength = 0; //���½����µ����ݰ�
    sprintf((char*)usart_send,"%s\r\n", cmd);
	send_len = strlen((char*)usart_send);
	esp01_uart_send_data(  usart_send , send_len );
    if(ack1==0 && ack2==0)     //����Ҫ��������
    {
		return true;
    }
    esp_delay_ms(time);   //��ʱ
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
    else if( ack1 != 0 )  //strstr(s1,s2);���s2�Ƿ�Ϊs1��һ���֣��Ƿ��ظ�λ�ã����򷵻�false����ǿ��ת��Ϊbool������
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack1 ) );

    else
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack2 ) );

}
/**
  * @brief   ѡ��ESP8266�Ĺ���ģʽ
  * @param   enumMode ģʽ���� STA AP STA_AP
  * @retval  ���� true ���ͳɹ���false ʧ��
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
  * @brief   ESP8266�����ⲿ��WIFI
  * @param   pSSID WiFi�ʺ�   pPassWord WiFi����
  * @retval  ���� true ���ͳɹ���false ʧ��
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
  * @brief   ESP8266���ö�����
  * @param   enumEnUnvarnishTx  �Ƿ�����ӣ�bool����
  * @retval  ���� true ���ͳɹ���false ʧ��
 **/
bool ESP8266_Enable_MultipleId (FunctionalState enumEnUnvarnishTx )
{
    char cStr [20];

    sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );

    return ESP8266_Send_AT_Cmd ( cStr, "OK", 0, 500 );

}

/**
  * @brief   ESP8266���� TCP ������  Ĭ�϶˿�Ϊ 333
  * @param   
  * @retval  ���� true ���ͳɹ���false ʧ��
 **/
bool ESP8266_Create_Tcp_server (  )
{
    char cStr [20];

    sprintf ( cStr, "AT+CIPSERVER=1" );

    return ESP8266_Send_AT_Cmd ( cStr, "OK", 0, 500 );

}

/**
	* @brief   ESP8266 ����һ������
	* @param   str ���������� uid Ҫ���͵Ķ���ID strlen ���������ݳ���
	* @retval  ���� true ���ͳɹ���false ʧ��
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
	* @brief   ESP8266 ���ӷ�����
	* @param   enumE  ��������
	* @param   ip ��������IP
	* @param   ComNum  �������˿�
	* @param   id�����Ӻţ�ȷ��ͨ�Ų���������
	* @retval  ���� true ���ͳɹ���false ʧ��
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
	* @brief   ͸��ʹ��
	* @retval  ���� true ���ͳɹ���false ʧ��
 **/
bool ESP8266_UnvarnishSend ( void )
{
    if (!ESP8266_Send_AT_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 ))
        return false;

    return 
        ESP8266_Send_AT_Cmd( "AT+CIPSEND", "OK", ">", 500 );
}

/**
	* @brief   ESP8266�����ַ���
	* @param   enumEnUnvarnishTx�Ƿ�ʹ��͸��ģʽ
	* @param   pStr�ַ���
	* @param   ulStrLength�ַ�������
	* @param   ucId ���Ӻ�
	* @retval  ���� true ���ͳɹ���false ʧ��
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
	* @brief   �˳�͸��
	* @retval  ���� true ���ͳɹ���false ʧ��
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
	* @brief   ESP8266 �������״̬
	* @retval  ����0����ȡ״̬ʧ��
	* @retval  ����2�����ip
	* @retval  ����3���������� 
	* @retval  ����4��ʧȥ���� 
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




