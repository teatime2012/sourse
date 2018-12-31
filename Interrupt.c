// gb2312

#include "FREDATA.h"

void MotorSpeedFunction(void);

extern FREState system; //exturn

interrupt void my_handler(void)
{
  static uint8_t LastDate, date, stepx, stepy, steplength;
  static uint8_t count1, stepCount;

  static uint16_t IR_temp;
  static uint8_t bitcount;

  if ((CCP2IE) && (CCP2IF)) //1 pulse    CCP1 RB4
  {
    //keyData++;
    MotorSpeedFunction(); //600r / min =  1 pulse in 1ms
    CCP2IF = 0;
  }
  if ((RCIE) && (RCIF)) //RX
  {
    RXQueueIn();
    //RCIF = 0; //USELESS
  }
  if ((IOCIF) && (IOCIE)) //
  {
    TMR4IF = 0;
    TMR4 = 105; //PERIOD = 844us // 8us * 105
    TMR4IE = 1;
    IR_temp = 0; // clear
    IOCIE = 0;   // IO interrupt
    IOCBF = 0;   //  IOCIF is read only, auto clear
  }
  if ((TMR4IF) && (TMR4IE))
  { //PERIOD = 1688us = 4t
    bitcount++;
    if (!RB5)
      IR_temp += 1; // recieve = !send  RB5 is IO
    //else  LED2 = OFF;
    if (bitcount >= 12)
    {
      bitcount = 0, system.IrData = IR_temp, TMR4IE = 0, IOCIE = 1, IOCBF = 0;
      //if(IrData == 0x0d02) HOLD = CLOSE;
      //if(IrData == 0x0d08) HOLD = CLOSE;
      //if(IrData == 0x0d20) HOLD = CLOSE;
    }
    else
      IR_temp <<= 1;
    TMR4IF = 0;
  }
  if ((TMR0IF) && (TMR0IE)) // 3ms cycle  16us * 195
  {
    TMR0 = 0X3D; //3120us
    count1++;
    if (count1 % 8 == 0) ////24.96ms
      system.mSecond_25++;
    if (system.keyCpHold == CLOSE)
      MotorFunction();
    TMR0IF = 0;
  }
}

#define SETMOVE 0 //½Ç¶È  +   Æ«ÒÆÁ¿
void MotorSpeedFunction(void)
{
  if (system.ccpState == CcpState_Clockwise) //1 0x05
  {
    if (CCP1 == 1) //count up
    {
      system.ccpNo = ccpAdd(system.ccpNo);
      if ((system.ccpNo == SETMOVE))
      {
        system.ccpState = CcpState_Clockwise_Back1;
        BREAK;
        CCP2CON = 0x04; //ÏÂ
      }
    }
  }
  else if (system.ccpState == CcpState_Clockwise_Back1) //2 0x04
  {
    REVERSE;
    if (CCP1 == 1)
    {
      //system.ccpNo = ccpMinus(system.ccpNo);
      system.ccpState = CcpState_AntiClockwise;
    }
    else
    {
      CCP2CON = 0x05; //up
      system.ccpState = CcpState_Clockwise_Back2;
    }
  }
  else if (system.ccpState == CcpState_Clockwise_Back2) //3 0x05
  {
    REVERSE;
    if (CCP1 == 1)
    {
      system.ccpNo = ccpAdd(system.ccpNo);
      system.ccpState = CcpState_Clockwise_Back1;
    }
    else
    {
      system.ccpNo = ccpAdd(system.ccpNo); //offset
      system.ccpState = CcpState_AntiClockwise;
    }
    CCP2CON = 0x04; //down
  }
  else if (system.ccpState == CcpState_AntiClockwise) //4 0x04
  {
    if (CCP1 == 1)
    {
      system.ccpNo = ccpMinus(system.ccpNo);
      if (system.ccpNo == 0)
      {
        system.ccpState = CcpState_AntiClockwise_Back1;
        BREAK;
        CCP2CON = 0x05; //up
      }
    }
  }
  else if (system.ccpState == CcpState_AntiClockwise_Back1) //5 0x05
  {
    FORWARD;
    if (CCP1 == 1)
    {
      //system.ccpNo = ccpAdd(system.ccpNo);
      system.ccpState = CcpState_Clockwise;
    }
    else
    {
      system.ccpState = CcpState_AntiClockwise_Back2;
      CCP2CON = 0x04; //
    }
  }
  else if (system.ccpState == CcpState_AntiClockwise_Back2) //6 0x04
  {
    FORWARD;
    if (CCP1 == 1)
    {
      system.ccpNo = ccpMinus(system.ccpNo);
      system.ccpState = CcpState_AntiClockwise_Back1;
    }
    else
    {
      system.ccpNo = ccpMinus(system.ccpNo); //offset
      system.ccpState = CcpState_Clockwise;
    }
    CCP2CON = 0x05; //
  }
}

uint8_t ccpAdd(uint8_t ccpNo)
{
  if (ccpNo >= 99)
    return 0;
  ccpNo++;
  return ccpNo;
}

uint8_t ccpMinus(uint8_t ccpNo)
{
  if (ccpNo == 0)
    return 99;
  ccpNo--;
  return ccpNo;
}
