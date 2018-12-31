#include "FREDATA.h" //gb2312

static uint8_t txQueueLen = 80;
uint8_t txQueue[80];
uint8_t txSendLen, txInPointer, txOutPointer;
static RxString_t rxString;

extern FREState system;
static uint8_t sleeping = 0;

void SleepGPIO(void);

void RxReset(void)
{
    memset(&rxString, 0, sizeof(rxString));
}

void TXQueueInitial(void)
{
    txSendLen = 0;
    txInPointer = 0;
    txOutPointer = 0;
    RxReset();
}

void TXQueueIn(uint8_t outData)
{
    if ((txSendLen < txQueueLen) && (sleeping == CLOSE))
    {
        txSendLen++;
        txQueue[txInPointer] = outData; //0
        txInPointer++;
        if (txInPointer >= txQueueLen) // cycle
            txInPointer = 0;
    }
}

void TXQueueOut(void)
{
    static uint8_t sleepCount = 0;
    if ((sleeping == OPEN) && (txSendLen == 0) && (TRMT == 1)) //test
    {
        sleepCount++;
        if (sleepCount >= 100)
        {
            sleepCount = 0;
            SleepGPIO();
        }
    }
    if ((txSendLen > 0) && (TRMT == 1)) // send over
    {
        txSendLen--;
        TXREG = txQueue[txOutPointer]; //0
        txOutPointer++;
        if (txOutPointer >= txQueueLen) //cycle
            txOutPointer = 0;
        if (sleeping)
            DelayMs(1);
    }
}

void RXQueueIn(void)
{
    static uint8_t temp, opCount;
    temp = RCREG; //get RXdata
    if (rxString.state == RxState_NULL)
    {
        if (temp == 0x77)
        {
            rxString.state = RxState_READING;
            rxString.length = 1;
            rxString.buffalo.checksum = 0x77;
        }
    }
    else if (rxString.state == RxState_READING)
    {
        rxString.length++;
        if (rxString.length == 2)
            rxString.buffalo.type = temp;
        else if (rxString.length == 3)
            rxString.buffalo.len = temp, opCount = 0;
        else if ((rxString.length > 3) && (opCount < rxString.buffalo.len))
        {
            if (opCount == 0)
                rxString.buffalo.opcode = temp;
            else
                rxString.buffalo.parameter[opCount - 1] = temp;
            opCount++;
        }
        else
        {
            if (rxString.buffalo.checksum == temp)
            {
                rxString.state = RxState_READY;
            }
            else
            {
                rxString.state = RxState_ERROR;
            }
        }
        rxString.buffalo.checksum ^= temp;
    }
}

void RXQueueOut(void)
{
    if (rxString.state == RxState_ERROR)
        RxReset();
    else if (rxString.state == RxState_READY)
    {
        if (rxString.buffalo.type == 0x04)
        {
            if (rxString.buffalo.opcode == 0x06) //上报数据 2字节手机地址
            {
                if (rxString.buffalo.parameter[2] == 0xAA) //first
                {
                    // HOLD = 0;
                    system.sleepFlag == OPEN; // lowpower
                    // if (rxString.buffalo.parameter[3] == 0x01)
                    //     bModulation = 1;
                    // if (rxString.buffalo.parameter[3] == 0x02)
                    //     bModulation = 0;
                    // if (rxString.buffalo.parameter[3] == 0x03)
                    //     cLedMode = 0;
                    // if (rxString.buffalo.parameter[3] == 0x04)
                    //     cLedMode = 1;
                    // if (rxString.buffalo.parameter[3] == 0x05)
                    //     cLedMode = 2;
                    // if (rxString.buffalo.parameter[3] == 0x06)
                    //     cLedMode = 3;
                    // if (rxString.buffalo.parameter[3] == 0x07)
                    //     cLedMode = 4;
                    // if (rxString.buffalo.parameter[3] == 0x08)
                    //     cLedMode = 5;
                }
            }
        }
        RxReset();
    }
}

#define BUFFALO_HEADER 0x77
void uart_send_package(uint8_t type, uint8_t para_len, uint8_t opcode, uint8_t *para)
{
    uint8_t output[32];
    uint8_t i = 0;
    uint8_t checksum = 0;
    uint8_t total_len = 0;
    total_len = para_len + 5;
    if (txSendLen + total_len > txQueueLen) //
        return;
    output[0] = BUFFALO_HEADER;
    output[1] = type;
    output[2] = para_len + 1;
    output[3] = opcode;
    memcpy(&output[4], para, para_len); //copy to
    for (i = 0; i < (total_len - 1); i++)
    {
        checksum = checksum ^ output[i];
        TXQueueIn(output[i]);
    }
    TXQueueIn(checksum);
    // for (i = 0; i < total_len; i++)
    // {
    //     SendData(output[i]); //test ok
    // }
}

void BLEPairing(float Interval, uint16_t Timeout)
{
    uint8_t Interval_H, Interval_L, Timeout_H, Timeout_L;
    uint8_t payload[4];
    uint8_t payload_len = 4;
    Interval = Interval / 0.625;
    Interval_L = (uint16_t)Interval & 0x00ff;
    Interval_H = ((uint16_t)Interval >> 8) & 0x00ff;
    Timeout_L = Timeout & 0x00ff;
    Timeout_H = (Timeout >> 8) & 0x00ff;
    payload[0] = Interval_L;
    payload[1] = Interval_H;
    payload[2] = Timeout_L;
    payload[3] = Timeout_H;
    uart_send_package(UART_PACKET_TYPE_CMD, payload_len, OPCODE_SET_LPM_ADV_PAIRING, payload);
}

void BLEName(void)
{
    uint8_t payload[2] = {84, 82};
    uint8_t payload_len = 2;
    uart_send_package(UART_PACKET_TYPE_CMD, payload_len, OPCODE_CHANGE_LOCAL_NAME, payload);
}

void BLEPower(void)
{
    uint8_t payload[1] = {7}; //7 = MAX
    uint8_t payload_len = 1;
    uart_send_package(UART_PACKET_TYPE_CMD, payload_len, OPCODE_LE_SET_TX_POWER, payload);
}

void BLEWakeupIO(void)
{
    uint8_t payload[2] = {0, 100};
    uint8_t payload_len = 2;
    uart_send_package(UART_PACKET_TYPE_CMD, payload_len, OPCODE_SET_LPM_WAKEUP_IO, payload);
}

void BLEConnect(void)
{
    uint8_t payload[8] = {0x20, 0x03, 0xC0, 0x03, 0x90, 0x01, 0xff, 0xff}; //0320 8小时
    uint8_t payload_len = 8;
    uart_send_package(UART_PACKET_TYPE_CMD, payload_len, OPCODE_SET_LPM_ADV_CONNECT, payload);
}

void BLE_Init(void)
{
    // BTRESET = 0;
    // __delay_ms(100);
    // BTRESET = 1;
    // __delay_ms(100);
    // uart_send_Byte(UART_PACKET_TYPE_CMD,OPCODE_GET_LOCAL_NAME);
    //    while (BLE_Adr[6] == 0x00)
    {
        //        RXData_Handle();
    }
    BLEName();
    BLEPower();
    BLEWakeupIO();
    // __delay_ms(100);
    BLEPairing(20, 90);
    // app_uart_rx_reset();
}

// void SendData(uint8_t x)
// {
//     TXREG = x;
//     while (TRMT == 0)
//         ; //sending
// }

void SleepMode(void)
{
    if ((system.sleepFlag == OPEN) && (sleeping == CLOSE))
    {
        BLEConnect();
        sleeping = OPEN;
    }
}

void SleepGPIO(void)
{
    //OSCCON = 0b01110010; // 8M   31 kHz LF 35uA
    GIE = 0;
    CCP1CON = 0;
    CCP2CON = 0;
    CCP5CON = 0;
    // TXSTA = 0x02; //off
    // RCSTA = 0;	//off
    TRISC = 0b10000000; // RC7:RX  RC6 TX
    PORTC = 0;          // Clear PORTC
    ADON = 0;
    TMR1ON = 0;
    TMR2ON = 0;
    TMR4ON = 0;
    TMR0IE = 0;
    CCP2IE = 0;
    RCIE = 0;   //-----------------------flag--------------
    ANSELA = 0; // AN0-5
    ANSELB = 0;
    ANSELD = 0;
    ANSELE = 0;
    TRISD = 0;
    PORTD = 0;
    TRISA = 0;          //00000000
    PORTA = 0;          //
    TRISB = 0b00000011; //RB0 IR, RB1 ONOFF, rb7-btio
    IOCBN = 0b00000010; // down check;
    PORTB = 0;          //
    TRISE = 0;          //
    PORTE = 0;          //
    HOLD = 1;           //100 uA
    DelayMs(5);
    TXQueueInitial();
    RotatorInitial();
    sleeping = CLOSE; //sleep done
    SLEEP();
    NOP();
    eepromSet();
    GPIO_Init(); //wake up
    system.longKeyFlag = OPEN;
}