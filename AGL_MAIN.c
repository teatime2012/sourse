/*************************
filename: AGL_MAIN.c
BUILD 171123    gb2312
***************************************************************************************************/
#include "FREDATA.h"
#include "STD.h"
#include <string.h>

__CONFIG(0x2b82); // 8M EXTERN-CLOCK
__CONFIG(0x18ff);
/*
__CONFIG(0x27a2);  //8007  out
__CONFIG(0x18ff);  //8008
*/

u8 keyFlag;

void InitialRotator(void)
{
	memset(&Rotator, 0, sizeof(RotatorState));
}

void motorOFF(void)
{
	if (CCP2IE == 1)
	{
		CCP2IE = 0;
		CCPR5L = 0;
	}
}

void motorON(void)
{
	if (Rotator.motorSpeed == MotorSpeed_0)
	{
		CCP2IE = 0;
		CCPR5L = 0;
	}
	else if (CCP2IE == 0)
	{
		CCP2IE = 1; // guang_ou
		CCPR5L = 30;
	}
}

void T2CPP15_init(void) // motor output
{						//T4CON = 0b00000110; // 8M/4 /16    8us; bit2 - enable
	//PR4 =  211; //
	//    CCP3SEL = 1;  // RB5:CCP3/P3A
	//    CCPR3L = 51; // %
	//    CCP3CON  = 0b00001100; // <3:0> = 1100 ,PWM ALL OPEN; <7:6> = 00, P3A only, from PxA
	//PWM模式下使用T2时钟 （直流电机）  CCP5 & CCP1
	CCPTMRS0 = 0; // all timer2
	CCPTMRS1 = 0; // default
	//T2CON = 0b01001110; // 8M/4 /10 /16 = 13kHz ; bit2 - enable
	T2CON = 0b00000100;   //      0.5 * 100 = 50us 20khz
	PR2 = 100;			  //  60ms /120 not use
	CCP5CON = 0b00001100; //CCP4/CCP5：11xx = PWM
	CCPR5L = 0;			  //CCPR5L  = 51;
	RIN = CLOSE;
	// RIN = OPEN;
	//  zhiliu dianji   ----  60hz
}

// 35 rc1 ccp2(1)
void T1CCP2_init(void) //capture the time of rotation  p213  // motor capture
{					   //CCP2做捕捉模式使用  只能使用T1
	CCP2CON = 0x07;	//0X05 - one   [3:0]0111 = 捕捉模式：每 16 个上升沿捕捉一次
	//CCP2CON = 0x05;		 //00000101
	APFCON = 0b00000000; //default
	CCP2SEL = 0;
	//TMR1IE = 1;
	//T1CON  = 0b00010001;    // FOSC/4 ,prescale = 2 1Mhz  1us;  bit0 - enable
	T1CON = 0b00110001; // FOSC/4 ,prescale = 8  f = 250k;  4us
						//CCPR2L = 0;  //timer counter, 100 pulse = 100ms(10hz) // CCPR2H = 0;
						// CCP2IE = 1; //
}
void T4IOCI_init(void)
{						//T4用在计算红外脉冲上，配合RB中断接口使用
	T4CON = 0b00000110; // 8M/4 /16    8us; bit2 - enable
	PR4 = 211;			//
	IOCIE = 1;			// IO interrupt
	IOCBN5 = 1;			// down check
						//IOCBP5  = 1;  // up check
}

#define TILTY1 RB1 //+Y
#define TILTY2 RB3 //-Y
#define TILTX2 RB0 //-X
#define TILTX1 RB2 //+X

int main()
{
	InitialRotator();
	Init();
	motorOFF();
	eepromSet();
	DOVCC = OPEN;
	while (1)
	{
#if 1
		KeyFunction();
		ADCheck();
		Vcheck();
		IRcheck();
		if (TILTX2 == 0)
			TiltFlagX2 = 0; // to gnd,tilt
		else
			TiltFlagX2 = 1;
		if (TILTY1 == 0)
			TiltFlagY1 = 0;
		else
			TiltFlagY1 = 1;
		if (TILTX1 == 0)
			TiltFlagX1 = 0;
		else
			TiltFlagX1 = 1;
		if (TILTY2 == 0)
			TiltFlagY2 = 0;
		else
			TiltFlagY2 = 1;
#endif
	}
	return 0;
}

void Init(void)
{
	//GPIO
	GIE = 0; //  8M out-os
	// OSCCON = 0b00001000; // 0b00111000  500k   0b01101000  4m
	ANSELA = 0b00000001; // AN0
	ANSELB = 0x00;
	ANSELD = 0x00; //  A/D select  1/1 - AN
	ANSELE = 0X03; //AD5,6

	TRISD = 0xc7;		//  11000111
	PORTD = 0x00;		//
	TRISA = 0xc1;		//  11000001
	PORTA = 0xff;		//
	TRISB = 0xFF;		//  11111111  除4,5可都用上拉  RB4 IN
	PORTB = 0x00;		// 00000000
	WPUB = 0xCF;		//   1 - enable
	TRISE = 0xFB;		// 11111011
	PORTE = 0x00;		// 00000000
	TRISC = 0b10010010; // 10010010   	// RC1 IN RC3,RC5 OUT
	PORTC = 0b00000000; // Clear PORTC

	ADCON1 = 0x00; // adc set  LEFT
	T1CCP2_init();
	T2CPP15_init();
	T4IOCI_init();
	//T0作为定时器使用
	OPTION_REG = 0X04; // 16us FOSC/4   PS<2:0>  1:2 -- 1:256  TMR0 Rate  1:32  100
	// RCIE = 1;

	TMR0IE = 1; // start without IIC fuction
	PEIE = 1;   // Enable peripheral interrupts
	GIE = 1;	// Enable global interrupts
	HOLD = CLOSE;
	LASER = OPEN;
}

void eepromSet(void)
{
#if 1
	XMotorOffset = (s8)(EEPROM_READ(0x01));
	YMotorOffset = (s8)(EEPROM_READ(0x02));
#else
	XMotorOffset = 0;
	YMotorOffset = 0;
#endif
}

#define IRMOTOR 50
void IRcheck(void)
{
	static u8 IrStep, IrTimer, IrCounter;
	static u16 IrSave, IrCompare;
	switch (IrStep)
	{
	case 0:
		if (IrData > 0)
		{
			if ((u8)(SecondCount - IrTimer) > 20)
				IrCounter = 0;
			IrTimer = SecondCount;
			IrStep++;
			IrSave = IrData, IrData = 0; // first code
		}
		break;
	case 1:
		if ((u8)(SecondCount - IrTimer) >= 2) // 一段时间内有效
			IrStep = 0, IrSave = 0;
		else if (IrData > 0)
		{
			if (IrData == IrSave)
				IrStep++;
			IrSave = IrData; // 有效信号
			IrData = 0;
		}
		break;
	case 2:
		if (IrSave == IrCompare)
		{
			if (IrCounter < 200)
				IrCounter++;
		}
		else
			IrCompare = IrSave, IrCounter = 0;
		switch (IrSave) //部分连发
		{
		case 0x0d02:
			keyFlag = 6;
			break; //6
		case 0x0d20:
			keyFlag = 4;
			break; //4
		case 0x0d08:
			if (Rotator.workState == WorkState_Manual)
				Rotator.yMotor = Motor_Up, Ycycle = IRMOTOR;
			else
				keyFlag = 5;
			break; //5
		case 0x0c81:
			keyFlag = 3;
			break; //3
		case 0x0c88:
			keyFlag = 2;
			break; //2
		case 0x0ca0:
			keyFlag = 1;
			break; //1
		case 0x0d04:
			if (Rotator.workState == WorkState_Manual)
				Rotator.yMotor = Motor_Down, Ycycle = IRMOTOR;
			else
				keyFlag = 9;
			break; //9
		case 0x0d10:
			if (Rotator.workState == WorkState_Manual)
				Rotator.xMotor = Motor_Up, Xcycle = IRMOTOR;
			else
				keyFlag = 7;
			break; //7
		case 0x0d01:
			if (Rotator.workState == WorkState_Manual)
				Rotator.xMotor = Motor_Down, Xcycle = IRMOTOR;
			else
				keyFlag = 8;
			break; //8
		/*      case 0x0e82: LEDON ^= 1; break;//111010000010
			    case 0x0f20: LEDON ^= 1; break;//111100100000
			    case 0x0f04: LEDON ^= 1; break;//111100000100
			    case 0x0f02: LEDON ^= 1; break;//111100000010*/
		case 0x0d0c: // y 5,9
			if (IrCounter == 10)
			{
				if (Rotator.workState == WorkState_Autoleveling)
					Rotator.workState = WorkState_Yadjust;
				else if (Rotator.workState == WorkState_Yadjust)
				{
					EEPROM_WRITE(0x02, YMotorOffset);
					Rotator.workState = WorkState_Autoleveling;
				}
			}
			break;
		case 0x0d11: //x 7,8
			if (IrCounter == 10)
			{
				if (Rotator.workState == WorkState_Autoleveling)
					Rotator.workState = WorkState_Xadjust;
				else if (Rotator.workState == WorkState_Xadjust)
				{
					EEPROM_WRITE(0x01, XMotorOffset);
					Rotator.workState = WorkState_Autoleveling;
				}
			}
			break;
		default:
			break;
		}
		IrSave = 0, IrStep++;
		IrTimer = SecondCount; // last code
		break;
	case 3:
		if (keyFlag > 0)
		{
			if (Rotator.workState == WorkState_Yadjust)
			{
				if ((keyFlag == 5) && (YMotorOffset < 120))
					YMotorOffset += 4;
				else if ((keyFlag == 9) && (YMotorOffset > (s8)(-120)))
					YMotorOffset -= 4;
				else if (keyFlag == 2)
				{
					YMotorOffset = 0;
				}
			}
			else if (Rotator.workState == WorkState_Xadjust)
			{
				if ((keyFlag == 7) && (XMotorOffset < 120))
					XMotorOffset += 4;
				else if ((keyFlag == 8) && (XMotorOffset > (s8)(-120)))
					XMotorOffset -= 4;
				else if (keyFlag == 2)
				{
					XMotorOffset = 0;
				}
			}
			else if (Rotator.workState == WorkState_Autoleveling)
			{
				if (keyFlag == 2)
				{
					Rotator.workState = WorkState_Manual;
					motorON(), LASER = OPEN;
				}
			}
			else if (Rotator.workState == WorkState_Manual)
			{
				if (keyFlag == 2)
				{
					Rotator.workState = WorkState_Autoleveling;
					motorOFF();
				}

				else if (keyFlag == 1)
				{
					if (Rotator.motorSpeed == MotorSpeed_600)
						Rotator.motorSpeed = MotorSpeed_0;
					else if (Rotator.motorSpeed == MotorSpeed_300)
						Rotator.motorSpeed = MotorSpeed_600;
					else if (Rotator.motorSpeed == MotorSpeed_0)
						Rotator.motorSpeed = MotorSpeed_300;
					motorON();
				}
			}
			keyFlag = 0;
		}
		IrStep++;
		break;
	default:
		IrStep = 0;
		break;
	}
}
