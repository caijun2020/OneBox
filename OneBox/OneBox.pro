#-------------------------------------------------
#
# Project created by QtCreator 2021-07-08T11:34:55
#
#-------------------------------------------------

QT       += core gui
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OneBox
TEMPLATE = app


SOURCES += App/main.cpp \
    App/MainWindow.cpp \
    Modbus/ModbusCommBase.cpp \
    Modbus/ModbusRTU/ModbusRTU.cpp \
    Modbus/ModbusRTU/ModbusRTUWidget.cpp \
    Modbus/ModbusTCP/ModbusTCP.cpp \
    Modbus/ModbusTCP/ModbusTCPWidget.cpp \
    TCPServer/TcpServerWidget.cpp \
    TCPServer/TcpServer.cpp \
    TCPClient/TcpClientWidget.cpp \
    TCPClient/TcpClient.cpp \
    UDPServer/UdpServerWidget.cpp \
    UDPServer/UdpServer.cpp \
    UDPClient/UdpClientWidget.cpp \
    UDPClient/UdpClient.cpp \
    SerialPort/SerialDebugWidget.cpp \
    SerialPort/QSerialPort.cpp \
    SerialPort/qextserialbase.cpp \
    Utility/CRC/CRCUtility.cpp \
    Utility/Log/FileLog.cpp \
    Utility/Buffer/LoopBuffer.cpp \
    Utility/Buffer/FifoBuffer.cpp \
    Utility/QUtilityBox.cpp

HEADERS  += App/MainWindow.h \
    Modbus/ModbusCommBase.h \
    Modbus/ModbusData.h \
    Modbus/ModbusRTU/ModbusRTU.h \
    Modbus/ModbusRTU/ModbusRTUWidget.h \
    Modbus/ModbusTCP/ModbusTCP.h \
    Modbus/ModbusTCP/ModbusTCPWidget.h \
    Modbus/endian_proc.h \
    TCPServer/TcpServerWidget.h \
    TCPServer/TcpServer.h \
    TCPClient/TcpClientWidget.h \
    TCPClient/TcpClient.h \
    UDPServer/UdpServerWidget.h \
    UDPServer/UdpServer.h \
    UDPClient/UdpClientWidget.h \
    UDPClient/UdpClient.h \
    SerialPort/SerialDebugWidget.h \
    SerialPort/QSerialPort.h \
    SerialPort/qextserialbase.h \
    SerialPort/ComInitData.h \
    Utility/CRC/CRCUtility.h \
    Utility/Log/FileLog.h \
    Utility/Buffer/LoopBuffer.h \
    Utility/Buffer/FifoBuffer.h \
    Utility/QUtilityBox.h \
    Utility/QtBaseType.h

FORMS    += App/MainWindow.ui \
    Modbus/ModbusRTU/ModbusRTUWidget.ui \
    Modbus/ModbusTCP/ModbusTCPWidget.ui \
    TCPServer/TcpServerWidget.ui \
    TCPClient/TcpClientWidget.ui \
    UDPServer/UdpServerWidget.ui \
    UDPClient/UdpClientWidget.ui \
    SerialPort/SerialDebugWidget.ui

win32{
    HEADERS += SerialPort/win_qextserialport.h
    SOURCES += SerialPort/win_qextserialport.cpp
}

unix{
    HEADERS += SerialPort/posix_qextserialport.h
    SOURCES += SerialPort/posix_qextserialport.cpp
}

RC_FILE = Resource/icon.rc

INCLUDEPATH += $$PWD/SerialPort
INCLUDEPATH += $$PWD/UDPServer
INCLUDEPATH += $$PWD/UDPClient
INCLUDEPATH += $$PWD/TCPServer
INCLUDEPATH += $$PWD/TCPClient
INCLUDEPATH += $$PWD/Modbus/ModbusRTU
INCLUDEPATH += $$PWD/Modbus/ModbusTCP
INCLUDEPATH += $$PWD/Modbus


INCLUDEPATH += $$PWD/Utility
INCLUDEPATH += $$PWD/Utility/Buffer
INCLUDEPATH += $$PWD/Utility/Log
INCLUDEPATH += $$PWD/Utility/CRC
LIBS +=

