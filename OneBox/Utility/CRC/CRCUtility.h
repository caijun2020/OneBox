/**********************************************************************
PACKAGE:        Utility
FILE:           CRCUtility.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Provide CRC calculation
**********************************************************************/

#ifndef CRCUTILITY_H
#define CRCUTILITY_H
#include <stdint.h>

class CRCUtility
{
public:
    CRCUtility();
    virtual ~CRCUtility();

    /*-----------------------------------------------------------------------
    FUNCTION:		instance
    PURPOSE:		Get a CRCUtility instance
    ARGUMENTS:		None
    RETURNS:		Return a static CRCUtility pointer
    -----------------------------------------------------------------------*/
    static CRCUtility *instance();

    /*-----------------------------------------------------------------------
    FUNCTION:		crc16
    PURPOSE:		calculate CRC-16
    ARGUMENTS:		const uint8_t *dataP, data buffer pointer
                    uint16_t length, data length
    RETURNS:		Return 16-bit crc value
    -----------------------------------------------------------------------*/
    uint16_t crc16(const uint8_t *dataP, uint16_t length);

    /*-----------------------------------------------------------------------
    FUNCTION:       modbus_crc16
    PURPOSE:        calculate CRC-16 for Modbus
    ARGUMENTS:      const uint8_t *dataP, data buffer pointer
                    uint16_t length, data length
    RETURNS:        Return 16-bit crc value
    -----------------------------------------------------------------------*/
    uint16_t modbus_crc16(const uint8_t *dataP, uint16_t length);
    
};

#endif // CRCUTILITY_H
