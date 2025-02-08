#include "bsp_key.h"

/**
  * @brief   ����ɨ�裬ѡ��ɨ���ģʽ��Ȼ�����һ��5ms��������
  * @param   key_scan_mode��SINGLE_SCAN ����ɨ��  CONTINUOUS_SCAN ����ɨ��
  * @retval
 **/
uint8_t KeyScan(const key_scan_mode_t key_scan_mode)
{
    static int keyCount = 0;
    static int keyState = 0;
    if(key_scan_mode == CONTINUOUS_SCAN) keyState = 0;
    if (keyState == 0 && (KEY1 == 0||KEY2 == 0 ))
    {    
        keyCount++;
        if(keyCount>2)
        {
            keyState = 1;
            keyCount=0;
            if(KEY1 == 0) return KEY1_VALUE;
            else if(KEY2 == 0) return KEY2_VALUE;
        }
    }else if (KEY1 == 1 && KEY2 == 1 )
    {
        keyState = 0;
    }
	return 0;
}

