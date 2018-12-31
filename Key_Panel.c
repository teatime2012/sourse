// gb2312
#include "FREDATA.h"
extern FREState system;

//-----------------KEY-----------------//
uint8_t getKey(void);
#define ONOFF_KEY 0xff //KEY上拉电阻  keydata为0
#define MANUAL_KEY 0x80
#define ROTATION_KEY 0xfe
#define ANGLE_KEY 0xfe
#define CLOSEWISE_KEY 0xfe
#define ANTICLOSEWISE_KEY 0xfe
#define TILT_KEY 0xfe
#define MOVEDETECT_KEY 0x80 //OK

#define KEY_LONGKEY 40
#define KEY_HOLD 3 //time delay

void KeyCheck(void)
{
    static uint8_t StepKEY, key, FirstKey;
    static uint8_t keyHoldCount, keyReleaseCount;

#if 1
    switch (StepKEY)
    {
    case 0:
        FirstKey = getKey(); // no key return 0xff
        if (FirstKey > 0)    // null key
        {
            keyReleaseCount = 0, keyHoldCount = 0;
            StepKEY++;
        }
        break;
    case 1:
        key = getKey();
        if (key == FirstKey)
        {
            keyReleaseCount = 0;
            if (keyHoldCount <= 200) // return
                keyHoldCount++;
        }
        else
        {
            keyReleaseCount++;
        }
        if (keyReleaseCount >= 3)
        {
            if (keyHoldCount <= 1) //check count
                FirstKey = 0;      //NULL
            StepKEY++;
        }
        if ((keyHoldCount > KEY_LONGKEY) && (system.longKeyFlag == CLOSE)) //long press function
        {
            // if ((system.workState == WorkState_Autoleveling_Y) && (TiltFlag.dat == 0x0f))
            // {
            //     // if (FirstKey == KEY1)
            //     //     system.alertKey = OPEN, system.xMotor = Motor_Up, Xcycle = 10;
            //     // if (FirstKey == KEY2)
            //     //     system.windKey = OPEN, system.xMotor = Motor_Down, Xcycle = 10;
            // }
            // if ((FirstKey == ONOFF_KEY) && (system.workState == WorkState_Null))
            // {
            //     system.workState = WorkState_Autoleveling; //start
            //     TiltCount = 0, LED3 = ON, HOLD = OPEN, system.longKeyFlag = 1;
            // } // bat green--3  bat red--2
            // else if ((keyHoldCount > KEY_HOLD) && (!system.longKeyFlag))
            // {
            //     if (FirstKey == KEY_COMBO)
            //     {
            //         LED5 = OFF, LED4 = OFF;
            //         if (system.workState == WorkState_Autoleveling)
            //         {
            //             memset(&system, 0, sizeof(systemState));
            //             system.workState = WorkState_Autoleveling_Y;
            //             system.xyMotorState = Motor_Xstop;
            //         }
            //         else if ((system.workState == WorkState_Autoleveling_Y))
            //         {
            //             memset(&system, 0, sizeof(systemState));
            //             system.workState = WorkState_Autoleveling;
            //         }
            //         else
            //             ;
            //         system.longKeyFlag = 1;
            //     }
            // }
            // if (FirstKey == MANUAL_KEY)
            // {
            //     // system.modeOverTime = CLOSE;
            //     if (system.workState == WorkState_Autoleveling)
            //         system.workState = WorkState_Manual;
            //     else if (system.workState == WorkState_Manual) //need else forbiden cycle
            //         system.workState = WorkState_Autoleveling;
            // }
            system.longKeyFlag = OPEN;
        }
        break;
    case 2:
        if (system.longKeyFlag == CLOSE)
        {
            if (FirstKey == ONOFF_KEY)
            {
                if (system.workState == WorkState_Null)
                {
                    system.workState = WorkState_Autoleveling;
                }
                else
                {
                    // HOLD = CLOSE;
                    system.sleepFlag = OPEN;
                }
            }
            if (FirstKey == MANUAL_KEY)
            {
                // system.modeOverTime = CLOSE;
                if (system.workState == WorkState_Autoleveling)
                    system.workState = WorkState_Manual;
                else if (system.workState == WorkState_Manual) //in alerting
                    system.workState = WorkState_Autoleveling;
            }
        }
        system.longKeyFlag = CLOSE;
        StepKEY = 0;
        break;
    default:
        StepKEY = 0;
        break;
    }
#else
    FirstKey = getKey();
    if (FirstKey > 0)
        StepKEY = FirstKey;
    if ((StepKEY > 0) && (FirstKey == 0))
    {
        if (StepKEY == MANUAL_KEY)
        {
            if (system.workState == WorkState_Autoleveling)
                system.workState = WorkState_Manual;
            else if (system.workState == WorkState_Manual) //in alerting
                system.workState = WorkState_Autoleveling;
        }
        // else
        //     system.workState = WorkState_Autoleveling;

        StepKEY = 0;
    }
#endif
}

//uint8_t KEY_SPI(void) // KeyDate for all
uint8_t getKey(void) // KeyDate for all
{
    uint8_t KeyDate; //static no need
    if (RB1 == 0)    //Onkey RB1
        KeyDate = ONOFF_KEY;
    else
    {
        system.keyCpHold = OPEN; //HOLD
        KEY_EN = ON;             //下降沿开始
        KeyDate = 0;
        NOP();
        NOP();
        NOP();
        NOP();
        KEY_EN = OFF;
        for (uint8_t i = 0; i < 8; i++) //Q1 -> Q7
        {
            ClockPulze = CLOSE;
            NOP();
            NOP();
            NOP();
            NOP();
            if (KEY_INPUT == ON) // 拉低为有效值 keydata反转
                KeyDate += 1;
            if (i < 7)
            {
                KeyDate <<= 1;
                ClockPulze = OPEN; // 上升沿开始
            }
        }
        system.keyCpHold = CLOSE;
    }
    return KeyDate;
}

uint8_t ccpMove(uint8_t ccpNo, uint8_t moveSteps)
{
}

// uint8_t getKey(void)
// {
//     static uint8_t key;
//     key = 0;
//     if (RB7 == 0)
//         key |= KEY1; //HI ALERT
//     if (RB6 == 0)
//         key |= KEY2; //WINDY
//     if (RD6 == 0)
//         key = ONOFF_KEY;
//     return key;
// }

//-------------led--------------//
void LED_SPI(uint8_t date) //595
{
    LED_EN = ON; //falling edge
    for (uint8_t i = 0; i < 8; i++)
    {
        LED_CP = CLOSE;
        NOP();
        NOP();
        NOP();
        NOP();
        if ((date & 0x80) == 0)
            LED_OUTPUT = 0;
        else
            LED_OUTPUT = 1;
        date <<= 1;
        LED_CP = OPEN;
    }
    LED_EN = OFF;
}

#define ONLED RD2
#define XRAY_LED system.ledData.Temp6       //X
#define YRAY_LED system.ledData.Temp0       //Y
#define ZRAY_LED system.ledData.Temp2       //Z
#define TILTKEY_LED system.ledData.Temp4    //
#define MOVEDETECT_LED system.ledData.Temp7 //LED4
#define SPEED_100_LED system.ledData.Temp5  //100
#define SPEED_400_LED system.ledData.Temp1  //400
#define SPEED_800_LED system.ledData.Temp3
void PanelFunction(void)
{ //LED,DIODE,SPEAKER,DC_MOTOR_INIT
    LedData = 0xff;
    // if (system.batteryState == Battery_Full)
    //     ONLED = ON;
    // else if (system.batteryState == Battery_Low)
    //     ONLED ^= 1;
    // if (system.tiltKeyFlag == OPEN)
    //     TILTKEY_LED = ON;
    // else
    //     TILTKEY_LED = OFF;
    // if (system.moveDelectFlag == OPEN)
    //     MOVEDETECT_LED = ON;
    // else
    //     MOVEDETECT_LED = OFF;
    if (system.rotateState == RotateState_800)
        SPEED_800_LED = ON;
    else if (system.rotateState == RotateState_400)
        SPEED_400_LED = ON;
    else if (system.rotateState == RotateState_100)
        SPEED_100_LED = ON;
    if (system.workState == WorkState_Manual)
    {
        if (system.zMode == OPEN) //ymode
            XRAY_LED = OFF, YRAY_LED = OFF, ZRAY_LED = ON;
        else
        {
            if (system.yMode == OPEN) //ymode
                XRAY_LED = OFF, YRAY_LED = ON, ZRAY_LED = OFF;
            else
                XRAY_LED = ON, YRAY_LED = OFF, ZRAY_LED = OFF;
        }
    }
    LED_SPI(LedData);
}