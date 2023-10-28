#include "keyscan.h"

#include "bsp_led.h"
#include "bsp_key.h"

u8 isKeyUp = 0;
u8 isKeyDown = 0;
u8 isKeyLeft = 0;
u8 isKeyRight = 0;
u8 isKeyOk = 0;


u16 keyOkCount = 0;
u8 keyPress = 0;

void KeyOk()
{
    keyPress=1;
    keyOkCount++;
}

void KeyLift()
{
    if(keyPress==1)
    {
        if(keyOkCount<20)
        {
            if(isKeyDown == 0)
                isKeyDown=1;
        } else if(keyOkCount<100)
        {
            if(isKeyRight == 0)
                isKeyRight=1;
        }
        keyOkCount=0;
        keyPress=0;
    }
}

/**
 * @brief 按键扫描函数
 *
 * @param mode 模式为1就是连续扫描，为0就是单次
 */
void key_scan(u8 mode)
{
    static int keyCount = 0;
    static int keyState = 0;
    if(mode == 1) keyState=0;
    if (keyState == 0 && (KEY_OK == 0))
    {
        keyCount++;
        if(keyCount>2)
        {
            keyState = 1;
            keyCount=0;
            if (KEY_OK == 0) KeyOk();
        }
    } else if (KEY_OK == 1)
    {
        keyState = 0;
        KeyLift();
    }
}

