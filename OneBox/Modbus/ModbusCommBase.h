#ifndef MODBUSCOMMBASE_H
#define MODBUSCOMMBASE_H

#include <QObject>
#include "ModbusData.h"


class ModbusCommBase : public QObject
{
    Q_OBJECT
public:
    explicit ModbusCommBase(QObject *parent = 0);
    virtual ~ModbusCommBase();
    
    /*-----------------------------------------------------------------------
    FUNCTION:       readHoldRegisters
    PURPOSE:        Read holding registers from modbusRTU slave device
    ARGUMENTS:      uint16_t regOffset  -- register offset address
                    uint16_t regCnt     -- count of registers
    RETURNS:        true - read successful, false - failed
    -----------------------------------------------------------------------*/
    virtual bool readHoldRegisters(uint16_t regOffset, uint16_t regCnt) = 0;

    /*-----------------------------------------------------------------------
    FUNCTION:       writeMultiRegisters
    PURPOSE:        Write multiple registers
    ARGUMENTS:      uint16_t regOffset      -- register offset address
                    const uint16_t *dataP   -- data pointer
                    uint16_t regCnt         -- count of registers
    RETURNS:        true - write successful, false - failed
    -----------------------------------------------------------------------*/
    virtual bool writeMultiRegisters(uint16_t regOffset, const char *dataP, uint16_t regCnt) = 0;

signals:
    void reportModbusResponseValue(struct MODBUS_READ_FEEDBACK s);

public slots:

protected:

protected:

};

#endif // MODBUSCOMMBASE_H
