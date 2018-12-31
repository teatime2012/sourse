// gb2312
#include "FREDATA.h"
extern FREState system;

int16_t getADChan(uint8_t channel)
{ //
	int16_t result;
	ADFM = 1;					 //right
	ADCON0 = (channel << 2) + 1; //nc:1 CHS<4.0>:5 , GO/DONW:1, ADON:1;
	asm("nop");
	asm("nop");
	ADGO = 1;
	while (ADGO)
		;
	result = ADRES; //
	return result;  //
}

void BattartCheck(void)
{
	static uint8_t lowCount, shutCount;
	static int16_t adVoltage;
	adVoltage = getADChan(5); // 3.3V system   6V/2
	if (adVoltage < 358)
		lowCount = 0, shutCount++; //1.4v
	else if (adVoltage < 405)
		lowCount++; // 1.6v
	else
		lowCount = 0, shutCount = 0;
	if (shutCount > 9)
		system.batteryState = Battery_Shut, HOLD = CLOSE; //shut
	else if (lowCount > 9)
		system.batteryState = Battery_Low;
	//led cycle
	// static uint8_t leMOTOR_OUTPUT tep, ledCount, filpFlag;
	// ledCycle = 2;
	// if (ledCycle > 0)
	// {
	// 	LED3 = OFF;
	// 	switch (leMOTOR_OUTPUT tep)
	// 	{
	// 	case 0:
	// 		if (ledCount < ledCycle)
	// 		{
	// 			if (!filpFlag)
	// 				LED1 = ON, filpFlag ^= 1;
	// 			else
	// 				LED1 = OFF, filpFlag ^= 1, ledCount++;
	// 		}
	// 		else
	// 			ledCount = 0, leMOTOR_OUTPUT tep++;
	// 		break;
	// 	case 1:
	// 		if (ledCount < 3)
	// 			ledCount++;
	// 		else
	// 			ledCount = 0, leMOTOR_OUTPUT tep++;
	// 		break;
	// 	default:
	// 		leMOTOR_OUTPUT tep = 0, ledCount = 0, filpFlag = 0;
	// 		break;
	// 	}
	// }
}

uint8_t XModulus(int16_t data)
{
	static int16_t minius;
	if (data > (512 + system.XMotorOffset))
		minius = data - 512 - system.XMotorOffset;
	else
		minius = 512 + system.XMotorOffset - data;

	if (minius > 200)
		return 100; // 0.5 / 3.3
	else if (minius > 65)
		return 10;
	else if (minius > 30)
		return 4;
	else if (minius > 5)
		return 1;
	else
		return 0;
}

uint8_t YModulus(int16_t data, int8_t offset)
{
	static int16_t minius;
	if (data > (512 + offset))
		minius = data - 512 - offset;
	else
		minius = 512 + offset - data;

	if (minius > 200)
		return 100; // 0.5 / 3.3
	else if (minius > 65)
		return 10;
	else if (minius > 30)
		return 4;
	else if (minius > 5)
		return 1;
	else
		return 0;
}

void ADCheck(void)
{
	static uint8_t StepAD, ADTimer;
	static uint8_t adXtemp, adYtemp; // adZtemp与Y共用
	static int16_t adX, adY;		 // not unsigned
	static uint8_t ADcount, WindTemp;
	static uint8_t bMotorTime;

	switch (StepAD)
	{
	case 0:
		if (system.workState <= WorkState_Autoleveling_Z) // auto
		{
			ADTimer = SecondCount;
			//			DOVCC = OPEN;
			StepAD++;
		}
		else
			//			DOVCC = CLOSE; // not use
			break;
	case 1:
		if (system.workState == WorkState_Autoleveling_Z)
			adY = getADChan(1); //Z水泡AD，共用Y马达
		else
		{
			adX = getADChan(2); //
			adY = getADChan(0); //
		}
		//DOVCC = CLOSE; // low consumption
		StepAD++; //break;
	case 2:
		if (system.workState == WorkState_Autoleveling_Z)
		{
			adYtemp = YModulus(adY, 0);
		}
		else
		{
			adXtemp = XModulus(adX);
			adYtemp = YModulus(adY, system.YMotorOffset);
		}
		if (system.levelingState == LevelingState_ING)
		{
			if ((adXtemp == 0) && (adYtemp == 0))
				ADcount++;
			else
				ADcount = 0;
			if (ADcount > 3) // anping
			{
				system.levelingState = LevelingState_Over;
				system.xMotor = Motor_Stop;
				system.yMotor = Motor_Stop;
				ADcount = 0; // faster
				StepAD++;
				break;
			}
		}
		else if (system.levelingState == LevelingState_Over) // stop return to leveling
		{
			if ((adXtemp != 0) || (adYtemp != 0))
				ADcount++;
			else
				ADcount = 0;

			// if (system.windMode)
			// 	WindTemp = 45; // windmode
			// else
			// 	WindTemp = 25;
			// if (system.alertMode)
			// 	WindTemp += 10;
			// if (ADcount > WindTemp)
			// {
			// 	system.modeOverTime = CLOSE; //
			// 	if (system.alertMode)
			// 		system.workState = WorkState_Alerting; //goto
			// 	system.levelingState = LevelingState_ING;
			// 	ADcount = 0;
			// }
			StepAD++;
			break;
		}
		if ((system.xyMotorState & Motor_Xstop) == 0) // in Z-mode,Xstop
		{
			if (adXtemp > 0)
			{
				if (adX < 512 + system.XMotorOffset) // x < VCC/2,  +x
				{
					system.xMotor = Motor_Up;
					if ((TiltFlagY1)) //& (TiltFlagY2) & (TiltFlagX1)) // FLAG1
						system.Xcycle = adXtemp;
					else
						system.Xcycle = 0;
				}
				else // -x
				{
					system.xMotor = Motor_Down;
					if ((TiltFlagY1) & (TiltFlagY2) & (TiltFlagX2)) // FLAG3
						system.Xcycle = adXtemp;
					else
						system.Xcycle = 0;
				}
				if (system.Xcycle < 50)
					bMotorTime = 1;
			}
		}
		if ((system.xyMotorState & Motor_Ystop) == 0)
		{ //0
			if (adYtemp > 0)
			{
				if (adY > 512 + system.YMotorOffset) // y > VCC/2,  +y
				{
					system.yMotor = Motor_Up;
					if ((TiltFlagX1) & (TiltFlagX2) & (TiltFlagY1)) // FLAG2
						system.Ycycle = adYtemp;
					else
						system.Ycycle = 0;
				}
				else
				{
					system.yMotor = Motor_Down;
					if ((TiltFlagX1) & (TiltFlagX2) & (TiltFlagY2)) // FLAG4
						system.Ycycle = adYtemp;
					else
						system.Ycycle = 0;
				}

				if (system.Ycycle < 50)
					bMotorTime = 1;
			}
		}
		StepAD++;
		break;
	//case 3: break;
	case 3:
		if (bMotorTime) // longer the HZ for motor
		{
			if ((uint8_t)(SecondCount - ADTimer) >= 2)
				bMotorTime = 0, StepAD = 0;
		}
		else
		{
			if ((uint8_t)(SecondCount - ADTimer) >= 1)
				bMotorTime = 0, StepAD = 0;
		}
		break;
	default:
		StepAD = 0;
		break;
	}
}

//-----------------motor-----------------
#if 0
#define QX1 0x08
#define QX2 0x01
#define QX3 0x04
#define QX4 0x02

#define QY1 0x80
#define QY2 0x10
#define QY3 0x40
#define QY4 0x20

#else if // change x,y
#define QY1 0x08
#define QY2 0x01
#define QY3 0x04
#define QY4 0x02

#define QX1 0x80
#define QX2 0x10
#define QX3 0x40
#define QX4 0x20

#endif

const uint8_t XSTEP[8] = {QX1, (QX1 + QX2), QX2, (QX3 + QX2), QX3, (QX3 + QX4), QX4, (QX1 + QX4)}; // 3ms nishizheng down
const uint8_t YSTEP[8] = {QY1, (QY1 + QY2), QY2, (QY3 + QY2), QY3, (QY3 + QY4), QY4, (QY1 + QY4)};
#define FASTSTEP 5

void Motor_SPI(uint8_t date) // DDRRD = DATE;
{
	MOTOR_EN = ON;
	for (uint8_t i = 0; i < 8; i++)
	{
		ClockPulze = CLOSE;
		if ((date & 0x80) == 0)
			MOTOR_OUTPUT = 0;
		else
			MOTOR_OUTPUT = 1;
		date <<= 1;
		ClockPulze = OPEN;
	}
	MOTOR_EN = OFF;
}

void MotorFunction(void)
{
	static uint8_t LastDate, date, stepx, stepy, steplength, stepCount;
	if ((system.xMotor == Motor_Stop) && (system.yMotor == Motor_Stop))
		Motor_SPI(0);
	else
	{
		date = 0x00;
		if (system.Xcycle > 0)
		{
			system.Xcycle--;
			if ((system.Xcycle > FASTSTEP))
				steplength = 2;
			else
				steplength = 1;
			if (system.xMotor == Motor_Up)
				stepx += steplength;
			else if (system.xMotor == Motor_Down)
				stepx -= steplength;
			stepx %= 8;
			date += XSTEP[stepx];
		}
		else
			system.xMotor = Motor_Stop;
		if (system.Ycycle > 0)
		{
			system.Ycycle--;
			if ((system.Ycycle > FASTSTEP))
				steplength = 2;
			else
				steplength = 1;
			if (system.yMotor == Motor_Up)
				stepy += steplength;
			else if (system.yMotor == Motor_Down)
				stepy -= steplength;
			stepy %= 8;
			date += YSTEP[stepy];
		}
		else
			system.yMotor = Motor_Stop;
		if (LastDate != date)
		{
			LastDate = date; //record
			Motor_SPI(date);
		}
	} //
}