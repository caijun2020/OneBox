#ifndef COM_INIT_DATA_H
#define COM_INIT_DATA_H

#include <QString>
#include "qextserialbase.h"

// Struct to store COM port init data
struct COM_PORT_INIT_DATA
{
    char port[20];
    BaudRateType baudrate;
    DataBitsType databits;
    ParityType parity;
    StopBitsType stopbits;
    FlowType flowtype;
};

#endif // COM_INIT_DATA_H
