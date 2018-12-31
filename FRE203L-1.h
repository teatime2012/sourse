// gb2312 ,181218
#ifndef __FRE203_H
#define __FRE203_H
/*
19  RA0   YCHECK
20  RA1   ZCHECK
21  RA2   XCHECK
22  RA3   MOTOR_EN 
23  RA4   KEY_EN  
24  RA5   VCHECK1
31  RA6   B3 
30  RA7   B2 

8   RB0   IR / INTERRUPT
9   RB1   ONKEY
10  RB2   LASER
11  RB3   BUBBLE
14  RB4   GUANG1
15  RB5   BT_EN
16  RB6    
17  RB7    
 
32  RC0    B4
35  RC1    GUANG2 / CCP2
36  RC2    RIN / CCP1
37  RC3    KEY_CP / MOTOR_CP
42  RC4    KEY_INPUT
43  RC5    MOTOR_OUTPUT
44  RC6    TX   DEVICE FOR BLUETOOTH
1   RC7    RX   DEVICE FOR BLUETOOTH

38  RD0    LED_EN
39  RD1    LED_CP
40  RD2    ONLED
41  RD3    LED_OUTPUT
2   RD4    B1
3   RD5    VCC_EN
4   RD6    HOLD
5   RD7    

25  RE0    TILT/Z
26  RE1    SPEAKER
27  RE2    FIN / CCP5
18  RE3     
*/

//SPI - MODULE
#define KEY_EN RA4       // ON
#define ClockPulze RC3   //O sck π≤”√ ±÷”
#define KEY_INPUT RC4    //I
#define MOTOR_OUTPUT RC5 // sdo
#define MOTOR_EN RA3

#define LED_EN RD0     //O
#define LED_CP RD1     //O
#define LED_OUTPUT RD3 //O

#define VCC_EN RD5
#define LASER RB2
#define HOLD RD6 //O
#define CCP1 RB4
#define CCP2 RC1 //CCP2
// #define RIN RC2  //PWM1 reverse
// #define FIN RE2  //PWM5 forward

#define FORWARD CCPR5L = 100, CCPR1L = 50 //clockwise
#define REVERSE CCPR5L = 50, CCPR1L = 100
//#define FORWARD CCPR5L = 100, CCPR1L = 0 //clockwise
//#define REVERSE CCPR5L = 0, CCPR1L = 100
#define BREAK CCPR5L = 100, CCPR1L = 100

//RB4   GUANG1
//RC1    GUANG2   CCP2

// #define Vcheck1 RD4 //I

// #define TILTY1 RB1 //+Y
// #define TILTY2 RB3 //-Y
// #define TILTX2 RB0 //-X
// #define TILTX1 RB2 //+X

// #define DOVCC RC0 //BUBBLE LASER

// #define SCIOPEN 0 //1 - open

void GPIO_Init(void);
void SleepMode(void);
void ADcheck(void);

void I2Cstep(void);
void Sendstep(void);
void WriteByte(uint8_t TxAddress, uint8_t TxData);
void ADCheck(void);
void DiodeFunction(void);
void motorON(void);
void motorOFF(void);
void IRcheck(void);
void eepromSet(void);
void DelayMs(uint16_t time);

void SleepMode(void);

uint8_t ccpAdd(uint8_t ccpNo);
uint8_t ccpMinus(uint8_t ccpNo);
void KeyCheck(void);
void BattartCheck(void);
void MotorFunction(void);
void PanelFunction(void);

void RotatorInitial(void);
void TXQueueInitial(void);
void TXQueueOut(void);
void RXQueueIn(void);
void RXQueueOut(void);
void Boardcast(void);
void BLE_Init(void);

#endif /*End File*/