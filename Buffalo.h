#ifndef __BUFFALO_H
#define __BUFFALO_H

typedef struct
{
	//uint8_t head = 0x77;
	uint8_t type;
	uint8_t len;
	uint8_t opcode;
	uint8_t parameter[20];
	uint8_t checksum;
} Buffalo_t;

typedef struct
{
	uint8_t state; // 0 -- ready  1 -- doing   2 -- OK   3 -- error
	Buffalo_t buffalo;
	uint8_t length;
} RxString_t;

//-------------------enum------------------------------//
typedef enum
{
	RxState_NULL = 0x00,
	RxState_READING = 0x01,
	RxState_READY = 0x02,
	RxState_ERROR,
} RxState_t; //接收状态就状态

typedef enum
{
	UARTIN_STATE_NONE = 0x00,
	UARTIN_STATE_READING = 0x01,
	UARTIN_STATE_CMD = 0x02,
} StateMachine_t; //接收状态就状态

typedef enum
{
	UART_PACKET_TYPE_CMD = 0X01,
	UART_PACKET_TYPE_RESERVED,
	UART_PACKET_TYPE_RSP,
	UART_PACKET_TYPE_EVENT,
} Buffalo_Type_t; //Type字节枚举

typedef enum
{
	OPCODE_SET_PAIRING_MODE = 0X01,
	OPCODE_SET_PAIRING_MODE_CANCEL,
	OPCODE_SET_DISCONNECTED,
	OPCODE_GET_LOCAL_NAME = 0x04,
	OPCODE_GET_LOCAL_ADDR,
	OPCODE_GET_FIRMWARE_VERSION = 0x06,
	OPCODE_SET_RED_LED,
	OPCODE_SET_YELLOW_LED,
	OPCODE_SEND_DATA_PACKET = 0x0b,
	OPCODE_SET_SEND_AUTO_DATA,
	OPCODE_SET_USER_SEND_DATA,
	OPCODE_CHANGE_LOCAL_NAME = 0x0c,
	OPCODE_GET_SYSTEM_STATE,
	OPCODE_SET_GPIO,
	OPCODE_SET_BAUDRATE,
	OPCODE_SET_SCAN,
	OPCODE_ENTER_DEEP_SLEEP,
	OPCODE_LE_SET_TX_POWER = 0XF3,
	OPCODE_SET_LPM_WAKEUP_IO = 0X12,
	OPCODE_SET_LPM_ADV_PAIRING = 0x13,
	OPCODE_SET_LPM_ADV_CONNECT = 0x14,
} Buffalo_Command_Opcode_t; //当Type为Command、Response时Opcode的意义

typedef enum
{
	EVENT_BLE_STARTUP = 0X01,
	EVENT_PARING_STATUS,
	EVENT_LOCAL_DEVICE_NAME,
	EVENT_LOCAL_BD_ADDRESS,
	EVENT_LOCAL_FIRMWARE_VERSION,
	EVENT_SEND_DATA_PACKET,
	EVENT_BAUDRATE_CHANGED,
	EVENT_ADV_REPORT,
	EVENT_LPM_ADV_TIMEOUT = 0x09,
} Buffalo_Event_Opcode_t; //当Type为Event时Opcode的意义

typedef enum
{
	RF_TX_POWER_GPIO,
	RF_TX_POWER_N14DB = 0x00,
	RF_TX_POWER_N11DB = 0x01,
	RF_TX_POWER_N8DB = 0x02,
	RF_TX_POWER_N5DB = 0x03,
	RF_TX_POWER_N2DB = 0x04,
	RF_TX_POWER_P2DB = 0x05,
	RF_TX_POWER_P4DB = 0x06,
	RF_TX_POWER_P8DB = 0x07,
} Buffalo_RF_Power_t; //Power枚举

typedef enum
{
	ERR_NONE = 0X01,
	ERR_LENGTH_FAIL,
	ERR_INVALID_FAIL,
} ErrorCode_t;

#endif
