//from FREDATA,  gb2312 ,181218
#ifndef __FREDATA_H //fredata
#define __FREDATA_H

#include <pic.h> //all pic
#include <string.h>
#include <stdint.h>

#include "Buffalo.h"
#include "FRE203L-1.h"

volatile uint8_t SecondCount;

typedef union //no type
{
  uint8_t me;
  struct
  {
    uint8_t Temp0 : 1;
    uint8_t Temp1 : 1;
    uint8_t Temp2 : 1;
    uint8_t Temp3 : 1;
    uint8_t Temp4 : 1;
    uint8_t Temp5 : 1;
    uint8_t Temp6 : 1;
    uint8_t Temp7 : 1;
  };
} CharUnion; // COMBOM 4 TILE FLAG

//----------------------FREState-----------------//
#define ON 0 //low level effective
#define OFF 1

typedef enum FlagState
{ //high level effective
  CLOSE = 0,
  OPEN = 1
} FlagState; //Type字节枚举

typedef enum
{
  WorkState_Autoleveling, //
  WorkState_Xadjust,
  WorkState_Yadjust,
  WorkState_Autoleveling_Z, //以上不触发adc
  WorkState_Manual,         //  手动模式
  WorkState_Sleep,
  WorkState_Null, //初始态//
} WorkState;      //Type字节枚举

typedef enum
{
  Battery_Full,
  Battery_Low,
  Battery_Shut
} BatteryState; //Type字节枚举

typedef enum
{ // 0 - xy all work
  Motor_Xstop = 1,
  Motor_Ystop = 2
} XYMotorState; //both motor

typedef enum
{
  Motor_Stop,
  Motor_Up,
  Motor_Down,
} MotorState; // single motor

typedef enum
{
  LevelingState_ING,
  LevelingState_Over,
  LevelingState_Outrange
} LevelingState;

typedef enum
{
  RotateState_800,
  RotateState_0,
  RotateState_100,
  RotateState_400,
  RotateState_angle1,
  RotateState_angle2,
  RotateState_angle3,
  RotateState_angle4
} RotateState;
/*  1         0      顺时针对比
  上升沿    下降沿    CCP2   顺：上升沿   逆:下降沿
    0         1      逆时针对比
1:vCC   2:GND   3:CCP    4: IO*/
typedef enum
{ //led
  CcpState_Clockwise,
  CcpState_Clockwise_Back1,
  CcpState_Clockwise_Back2,
  CcpState_AntiClockwise,
  CcpState_AntiClockwise_Back1,
  CcpState_AntiClockwise_Back2,
} CcpState; //Type字节枚举

typedef struct
{
  uint8_t mSecond_25;
  int8_t XMotorOffset, YMotorOffset;     //read from EEPROM
  CharUnion tiltFlag;                    //tilt    inputdata
  FlagState yMode, zMode;                //Z-MODE flag
  FlagState tiltKeyFlag, moveDelectFlag; //for led
  uint16_t IrData;                       //ir  adc  blue
  FlagState longKeyFlag, keyCpHold;
  CcpState ccpState;          //StateData ---
  uint8_t ccpNo;              // ccpLocation
  uint8_t ccpHead, ccpBottom; //ccpRange
  WorkState workState;
  BatteryState batteryState;
  XYMotorState xyMotorState; //can stop ADC
  MotorState motorState;     //can change by key, step
  LevelingState levelingState;
  RotateState rotateState; // speed or angle
  MotorState xMotor, yMotor;
  //output data  Laser Speak  , DC: pwmOut
  uint8_t Xcycle, Ycycle; // motor
  CharUnion ledData;
  FlagState sleepFlag;
} FREState; // system

#define LedData system.ledData.me
#define TiltFlag system.tiltFlag.me
#define TiltFlagX1 system.tiltFlag.Temp0
#define TiltFlagX2 system.tiltFlag.Temp1
#define TiltFlagY1 system.tiltFlag.Temp2
#define TiltFlagY2 system.tiltFlag.Temp3

#endif