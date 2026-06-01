/**********************************************************************
PACKAGE:        Utility
FILE:           CRCUtility.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        CRC calculation
**********************************************************************/

#include "CRCUtility.h"

CRCUtility::CRCUtility()
{
}

CRCUtility::~CRCUtility()
{
}

CRCUtility* CRCUtility::instance()
{
    static CRCUtility singleton;
    return &singleton;
}

uint16_t CRCUtility::crc16(const uint8_t *dataP, uint16_t length)
{
    uint16_t  CrcReg = 0xffff;

    for(int i = 0; i < length; i++)
    {
        uint8_t BArrayData = *(dataP + i);
        CrcReg = ((BArrayData ^ CrcReg) & 0x00ff) | (CrcReg & 0xff00);
        for(int j = 0; j < 8; j++)
        {
            uint8_t bLsb = CrcReg & 0x0001;
            CrcReg = (CrcReg >> 1) & 0x7fff;
            if(bLsb == 0x0001)
            {
                CrcReg = CrcReg ^ 0xA001;
            }
        }
    }

    return CrcReg;
}

uint16_t CRCUtility::modbus_crc16(const uint8_t *dataP, uint16_t length)
{
    int cr = 0xFFFF;
    
    for(int i = 0; i < length; i++)
    {
        cr = cr ^ dataP[i];
        for(int j = 0; j < 8; j++)
        {
            if ((cr & 0x0001) == 1)
            {
                cr >>= 1;
                cr = cr ^ 0xA001;
            }
            else
            {
                cr >>= 1;
            }
        }
    }
    
    return cr;
}

