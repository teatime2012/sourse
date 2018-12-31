// gb2312

#include "FREDATA.h"

__CONFIG(0x2982); // 8M-inTERN config   browm - out off 内含LDO
__CONFIG(0x18ff);

uint8_t ledData;

FREState system;

void MainStep(void)
{
	static uint8_t pointer, mainTimer;  //static error
	if ((pointer != system.mSecond_25)) //
	{
		pointer = system.mSecond_25;
		if (mainTimer < 200) // 25ms * 100 = 5s
		{
			KeyCheck();
			if (mainTimer % 10 == 0) //250ms
			{
				//	BattartCheck();
			}
			if (mainTimer % 20 == 0) //500ms
			{
				PanelFunction();
				SleepMode();
			}
			mainTimer++;
		}
		else
			mainTimer = 0;
	}
}

void RotatorInitial(void)
{
	memset(&system, 0, sizeof(FREState));
}

int main()
{
	RotatorInitial();
	GPIO_Init();
	eepromSet();
	BLE_Init();						   //
	system.workState = WorkState_Null; //
	while (1)
	{
		MainStep();
		//ADC();
		//ir();
		RXQueueOut();
		TXQueueOut();
	}
	return 0;
}

void T2CPP15_init(void) // motor output
{
	//PWM模式下使用T2时钟 （直流电机）  CCP5 & CCP1
	CCPTMRS0 = 0;		// all timer2
	CCPTMRS1 = 0;		// default
	T2CON = 0b00000100; //  0.5 * 100 = 50us 20khz  bit2 - enable
	PR2 = 100;			//  60ms /120 not use
	CCPR1L = 100;
	CCPR5L = 100;
	CCP5CON = 0b00001100; //CCP4/CCP5：11xx = PWM
	CCP1CON = 0b00001100; // PWM模式  单输出
}

// 35 rc1 ccp2(1)
void T1CCP2_init(void)   //capture the time of rotation  p213  // motor capture
{						 //CCP2做捕捉模式使用  只能使用T1
	CCP2CON = 0x05;		 //0X05 - one   [3:0]0111 = 捕捉模式：每 16 个上升沿捕捉一次
	APFCON = 0b00000000; //default
	CCP2SEL = 0;
	//T1CON  = 0b00010001;    // FOSC/4 ,prescale = 2 1Mhz  1us;  bit0 - enable
	T1CON = 0b00110001; // FOSC/4 ,prescale = 8  f = 250k;  4us
	CCP2IE = 1;			// 开中断
}

void EUSART_init(void) //P305
{
	TXSTA = 0x24; // 0010 0100  异步 bit5:使能发送 bit2:BRGH 高波特率选择位 bit1:TRMT 1 = TSR 为空
	RCSTA = 0x90; // 1001 0000  7:SPEN = 1 4:CREN = 1 使能串口,接收
	BRG16 = OPEN; //十六位波特率发生器  //ABDEN = OPEN;         //auto-baud rate
	SPBRG = 207;  // = 207 :9600   = 16 :115200
	RCIE = 1;	 //    接收中断
}

void T4IOCI_init(void)
{						//T4用在计算红外脉冲上，配合RB中断接口使用
	T4CON = 0b00000110; // 8M/4 /16    8us; bit2 - enable
	PR4 = 211;			//
	IOCIE = 1;			// IO interrupt
	IOCBN = 0b00000001; // down check; RB0 IR, RB1 ONOFF, rb7-btio
						//IOCBP5  = 1;  // up check
}

void GPIO_Init(void) //
{
	GIE = 0;
	OSCCON = 0b01110010; // 内部时钟配置 8M  默认500k 外部见config  bit1 = 使用内部振荡器模块
	ANSELA = 0b00100111; // AN0-2 5
	ANSELB = 0x00;
	ANSELD = 0x00;		//  A/D select  1/1 - AN
	ANSELE = 0X00;		//
	TRISD = 0b00010000; //
	PORTD = 0b01100000; //hold = 1, vccen = 1
	TRISA = 0b11100111; //00000000
	PORTA = 0;			//
	TRISB = 0b00010011; // RB0 IR, RB1 ONOFF, rb7-btio
	PORTB = 0x00;		//
	//WPUB = 0xCF;		//
	TRISE = 0b00000001; //
	PORTE = 0x00;		//
	TRISC = 0b10010011; // RC1 CCP I   RC7:RX  RC6 TX
	PORTC = 0b00000000; // Clear PORTC
	ADCON1 = 0x00;		// adc set  LEFT
	TXQueueInitial();
	EUSART_init();
	T1CCP2_init();
	T2CPP15_init();
	T4IOCI_init();
	//T0作为定时器使用
	OPTION_REG = 0X04; // 16us FOSC/4   PS<2:0>  1:2 -- 1:256  TMR0 Rate  1:32  100
	TMR0IE = 1;		   // start without IIC fuction
	PEIE = 1;		   // Enable peripheral interrupts
	GIE = 1;		   // Enable global interrupts
}

void eepromSet(void)
{
	system.XMotorOffset = (int8_t)(EEPROM_READ(0x01));
	system.YMotorOffset = (int8_t)(EEPROM_READ(0x02));
}

// void KeyCheck(void)
// {
// 	static uint8_t keySave, keyCount;
// 	if (ONKEY) // ??
// 	{
// 		if (keySave != 0xff)
// 			keyCount = 0;
// 		keySave = 0xff;
// 		keyCount++;
// 	}
// 	else
// 	{
// 		KEY_SPI();
// 		if (keySave == KeyDate)
// 			keyCount++;
// 		else
// 			keyCount = 0, keySave = KeyDate;
// 	}
// 	if (keyCount >= 3)
// 	{
// 		keyCount = 0;
// 		if (keySave == 0xff)
// 		{
// 		}
// 	}
// 	if (KeyDate == KEY1)
// 	{
// 	}
// }

void SystemTicks250(void);
void SystemTicks500(void);
void SystemTicks1000(void);

void DelayMs(uint16_t time)
{
	for (uint16_t i = 0; i < time; i++)
		for (uint16_t ii = 0; ii < 1024; ii++)
			NOP();
}
