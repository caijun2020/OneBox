#ifndef MODBUS_DATA_STRUCT_H
#define MODBUS_DATA_STRUCT_H

#include <stdint.h>

#define MODBUS_CRC_LENGTH 2

#define MODBUS_RESPONSE_MSG_START_LEN 6
#define MODBUS_RESPONSE_MSG_START_LEN_2 2
#define MODBUS_RESPONSE_MSG_START_LEN_4 4


#define MB_ADDRESS_BROADCAST    ( 0 )   /*! Modbus broadcast address. */
#define MB_ADDRESS_MIN          ( 1 )   /*! Smallest possible slave address. */
#define MB_ADDRESS_MAX          ( 247 ) /*! Biggest possible slave address. */
#define MB_FUNC_NONE                          (  0 )
#define MB_FUNC_READ_COILS                    (  1 )
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
#define MB_FUNC_WRITE_SINGLE_COIL             (  5 )
#define MB_FUNC_WRITE_MULTIPLE_COILS          ( 15 )
#define MB_FUNC_READ_HOLDING_REGISTER         (  3 )
#define MB_FUNC_READ_INPUT_REGISTER           (  4 )
#define MB_FUNC_WRITE_REGISTER                (  6 )
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS      ( 16 )
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS  ( 23 )
#define MB_FUNC_DIAG_READ_EXCEPTION           (  7 )
#define MB_FUNC_DIAG_DIAGNOSTIC               (  8 )
#define MB_FUNC_DIAG_GET_COM_EVENT_CNT        ( 11 )
#define MB_FUNC_DIAG_GET_COM_EVENT_LOG        ( 12 )
#define MB_FUNC_OTHER_REPORT_SLAVEID          ( 17 )
#define MB_FUNC_ERROR                         ( 128 )

typedef enum
{
    MODBUS_WR_OPT = 0,
    MODBUS_RD_OPT = 1
}MODBUS_RD_WR_OPT;

struct MODBUS_READ_FEEDBACK
{
    uint16_t address;       // Reg address
    uint16_t buffer[256];
    uint16_t len;           // Reg count, len= (active size of buffer[])/2
    uint16_t rdwrFlag;      // 0: write, 1: read
};

typedef enum{
    MODBUS_ADDR_BROADCAST = 0x00
}MODBUS_ADDR;

typedef enum{
    FUNCTION_CODE_SERVICE_SESSION = 0X64,

    FUNCTION_CODE_CONFIGURATION_SESSION = 0X66,
    FUNCTION_CODE_SUBCODE_OPEN = 0x00,
    FUNCTION_CODE_SUBCODE_MFG_TEST = 0x11,
    FUNCTION_CODE_GATEWAY_MODE = 0xFF,

    FUNCTION_CODE_READ_HOLD_REG = 0X03,
    FUNCTION_CODE_READ_HOLD_REG_ERR = 0X83,

    FUNCTION_CODE_WRITE_MULTI_REG = 0X10,
    FUNCTION_CODE_WRITE_MULTI_REG_ERR = 0X90
}MODBUS_FUNCTION_CODE;


enum{
    BIT_0 = 0,
    BIT_1 = 1,
    BIT_2 = 2,
    BIT_3 = 3,
    BIT_4 = 4,
    BIT_5 = 5,
    BIT_6 = 6,
    BIT_7 = 7,
    BIT_8 = 8,
    BIT_9 = 9,
    BIT_10 = 10,
    BIT_11 = 11,
    BIT_12 = 12,
    BIT_13 = 13,
    BIT_14 = 14,
    BIT_15 = 15
};

struct MODBUS_RX_MSG_STRUCT
{
    uint8_t address;
    uint8_t functionCode;
    uint8_t data[256];
    uint8_t len;    // indicate the active lenght of address+functionCode+data
};

enum
{
    PRINT_PASS = 1,
    PRINT_FAIL = 2
};

#endif // MODBUS_DATA_STRUCT_H
