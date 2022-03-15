/**********************************************************************
PACKAGE:        Communication
FILE:           SerialDebugWidget.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Serial Port Debug Widget UI
**********************************************************************/

#include "SerialDebugWidget.h"
#include "ui_SerialDebugWidget.h"
#include "qextserialbase.h"
#include "QUtilityBox.h"
#include "QtBaseType.h"
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

SerialDebugWidget::COMBOX_LIST baudrate_combox[] =
{
    {BAUD9600, "9600"},
    {BAUD19200, "19200"},
    {BAUD38400, "38400"},
    {BAUD57600, "57600"},
    {BAUD115200, "115200"}
};

SerialDebugWidget::COMBOX_LIST parity_combox[] =
{
    {PAR_NONE, "None"},
    {PAR_ODD, "Odd"},
    {PAR_EVEN, "Even"},
    {PAR_MARK, "Mark"},
    {PAR_SPACE, "Space"}
};

SerialDebugWidget::COMBOX_LIST dataBits_combox[] =
{
    {DATA_5, "5"},
    {DATA_6, "6"},
    {DATA_7, "7"},
    {DATA_8, "8"}
};

SerialDebugWidget::COMBOX_LIST stopBits_combox[] =
{
    {STOP_1, "1"},
    {STOP_1_5, "1.5"},
    {STOP_2, "2"}
};

SerialDebugWidget::COMBOX_LIST flowType_combox[] =
{
    {FLOW_OFF, "Off"},
    {FLOW_HARDWARE, "Hardware"},
    {FLOW_XONXOFF, "XonXoff"}
};


SerialDebugWidget::SerialDebugWidget(QSerialPort *portHandler, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialDebugWidget),
    m_settingFile("config.ini"),
    serialPort(portHandler),
    comInitData(new struct COM_PORT_INIT_DATA),
    widgetFontType("Arial"),
    widgetFontSize(10),
    isOpenFlag(false),
    timeStampFlag(false),
    hexFormatFlag(true),
    autoClearRxFlag(true),
    refreshTimer(new QTimer),
    refreshInMs(1000),
    m_title(""),
    showTxPacketFlag(true),
    showRxPacketFlag(true)
{
    ui->setupUi(this);

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    // Prepend the exe absolute path
    m_settingFile.prepend(QUtilityBox::instance()->getAppDirPath());

    // Default setting file
    currentSetting = new QSettings(m_settingFile, QSettings::IniFormat);

    // Load Settings from ini file
    loadSettingFromIniFile();

    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    refreshTimer->start(refreshInMs);  //1s

    connect(&autoSendTimer, SIGNAL(timeout()), this, SLOT(autoSendData()));

    // Set Window Title
    this->setWindowTitle(tr("Serial Port Debug Widget"));
}

SerialDebugWidget::~SerialDebugWidget()
{
    delete ui;
    delete comInitData;
    delete currentSetting;
    delete refreshTimer;
}

void SerialDebugWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    QWidget *pWidget = static_cast<QWidget*>(this->parent());

    if(pWidget != NULL)
    {
        this->resize(pWidget->size());
    }
}

void SerialDebugWidget::initWidgetFont()
{
#if 0
    // Init Font Size and bold
    ui->label_port->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->label_baudrate->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->label_parity->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->label_dataBits->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->label_stopBits->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->label_flowControl->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));

    ui->comboBox_port->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->comboBox_baudrate->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->comboBox_parity->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->comboBox_dataBits->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->comboBox_stopBits->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->comboBox_flowControl->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));

    ui->pushButton_open->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    //ui->pushButton_clear->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    //ui->pushButton_reset->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->pushButton_send->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));

    //ui->checkBox_hex->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    //ui->checkBox_timeStamp->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));

    ui->label_autoSend->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->label_ms->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    ui->spinBox_autoSendInterval->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));

    //ui->label_rx->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    //ui->label_rxBytes->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    //ui->label_tx->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
    //ui->label_txBytes->setFont(QFont(widgetFontType, widgetFontSize, QFont::Normal));
#endif
}

void SerialDebugWidget::initWidgetStyle()
{
    ui->spinBox_autoSendInterval->setRange(1, 1000000);
    ui->spinBox_autoSendInterval->setValue(1000);

    QStringList comPorts = QSerialPort::getAvailablePorts();
    ui->comboBox_port->clear();
    ui->comboBox_port->insertItems(0, comPorts);

    for(uint32_t i = 0; i < (sizeof(baudrate_combox) / sizeof(COMBOX_LIST)); i++)
    {
        ui->comboBox_baudrate->insertItem(i, baudrate_combox[i].text);
    }
    ui->comboBox_baudrate->setCurrentIndex(4);

    for(uint32_t i = 0; i < (sizeof(parity_combox) / sizeof(COMBOX_LIST)); i++)
    {
        ui->comboBox_parity->insertItem(i, parity_combox[i].text);
    }

    for(uint32_t i = 0; i < (sizeof(dataBits_combox) / sizeof(COMBOX_LIST)); i++)
    {
        ui->comboBox_dataBits->insertItem(i, dataBits_combox[i].text);
    }
    ui->comboBox_dataBits->setCurrentIndex(3);

    for(uint32_t i = 0; i < (sizeof(stopBits_combox) / sizeof(COMBOX_LIST)); i++)
    {
        ui->comboBox_stopBits->insertItem(i, stopBits_combox[i].text);
    }

    for(uint32_t i = 0; i < (sizeof(flowType_combox) / sizeof(COMBOX_LIST)); i++)
    {
        ui->comboBox_flowControl->insertItem(i, flowType_combox[i].text);
    }

    if(NULL != serialPort)
    {
        QStringList comPorts = serialPort->getAvailablePorts();
        ui->comboBox_port->clear();
        ui->comboBox_port->insertItems(0, comPorts);
    }

    ui->checkBox_timeStamp->setChecked(timeStampFlag);
    ui->checkBox_autoClear->setChecked(autoClearRxFlag);
    ui->checkBox_hex->setChecked(hexFormatFlag);

    ui->checkBox_showRx->setChecked(showRxPacketFlag);
    ui->checkBox_showTx->setChecked(showTxPacketFlag);
}

void SerialDebugWidget::on_pushButton_send_clicked()
{
    uint32_t txLen = 0;
    uint8_t tempBuf[10000] = {0};
    QByteArray tempTxBuf;
    QUtilityBox utilityBox;

    // Hex format
    if(hexFormatFlag)
    {
        txLen = utilityBox.convertHexStringToDataBuffer(tempBuf, ui->textEdit_txData->toPlainText());
        tempTxBuf = QByteArray((char *)tempBuf, txLen);
    }
    else
    {
        tempTxBuf = ui->textEdit_txData->toPlainText().toLocal8Bit();
    }

    // Send out data
    sendData(tempTxBuf);
}

void SerialDebugWidget::bindModel(QSerialPort *portHandler)
{
    if(NULL != portHandler)
    {
        unbind();

        serialPort = portHandler;
        connect(serialPort, SIGNAL(newDataReady(QByteArray)), this, SLOT(updateIncomingData(QByteArray)));
        connect(serialPort, SIGNAL(statusChanged(COM_PORT_INIT_DATA *)), this, SLOT(updateConnectionStatus(COM_PORT_INIT_DATA*)));

        // Only serial port is open, then update status(Buadrate/Databits/Stopbits...)
        if(serialPort->isOpen())
        {
            serialPort->notifyStatusChanged();
        }
        else
        {
            // Open com port with delay
            QTimer::singleShot(refreshInMs, this, SLOT(on_pushButton_open_clicked()));
        }
    }
}

void SerialDebugWidget::unbind()
{
    if(NULL != serialPort)
    {
        disconnect(serialPort, 0 , this , 0);
    }

    serialPort = NULL;
}

void SerialDebugWidget::sendData(QByteArray &data)
{
    sendData(data.data(), data.length());
}

void SerialDebugWidget::sendData(const char *data, uint32_t len)
{
    if(NULL != serialPort)
    {
        serialPort->writeData(data, len);
    }
}

void SerialDebugWidget::on_pushButton_open_clicked()
{
    QString warningStr = tr("Open %1 failed!").arg(ui->comboBox_port->currentText());

    if(NULL == serialPort)
    {
        return;
    }

    // Clear init data struct
    memset(comInitData, 0, sizeof(struct COM_PORT_INIT_DATA));

    memcpy(comInitData->port, ui->comboBox_port->currentText().toLatin1().data(), ui->comboBox_port->currentText().toLatin1().length());
    comInitData->baudrate = static_cast<BaudRateType>(baudrate_combox[ui->comboBox_baudrate->currentIndex()].value);
    comInitData->parity = static_cast<ParityType>(parity_combox[ui->comboBox_parity->currentIndex()].value);
    comInitData->databits = static_cast<DataBitsType>(dataBits_combox[ui->comboBox_dataBits->currentIndex()].value);
    comInitData->stopbits = static_cast<StopBitsType>(stopBits_combox[ui->comboBox_stopBits->currentIndex()].value);
    comInitData->flowtype = static_cast<FlowType>(flowType_combox[ui->comboBox_flowControl->currentIndex()].value);

    isOpenFlag = serialPort->isOpen();

    // If port is closed, then open it
    if(!isOpenFlag)
    {
        serialPort->open(comInitData);

        isOpenFlag = serialPort->isOpen();
        if(!isOpenFlag)
        {
            QMessageBox::information(this, tr("Notice"), warningStr);
        }
    }
    else
    {
        serialPort->close();
    }

    // Update setting to file
    updateSettingToFile();
}

void SerialDebugWidget::setFunctionUI(bool enable)
{
    ui->comboBox_port->setEnabled(enable);
    ui->comboBox_baudrate->setEnabled(enable);
    ui->comboBox_parity->setEnabled(enable);
    ui->comboBox_dataBits->setEnabled(enable);
    ui->comboBox_stopBits->setEnabled(enable);
    ui->comboBox_flowControl->setEnabled(enable);
}

void SerialDebugWidget::updateUI()
{
    if(NULL == serialPort)
    {
        return;
    }

    ui->label_rxBytes->setText(QString::number(serialPort->getTotalRxBytes()));
    ui->label_txBytes->setText(QString::number(serialPort->getTotalTxBytes()));
}

void SerialDebugWidget::on_pushButton_reset_clicked()
{
    if(NULL == serialPort)
    {
        return;
    }

    serialPort->resetTxRxCnt();
}

void SerialDebugWidget::on_pushButton_clear_clicked()
{
    ui->textEdit_rxData->clear();
}

void SerialDebugWidget::updateIncomingData(QByteArray data)
{
    QString dataStr;
    QUtilityBox utilityBox;

    dataStr.clear();

    // Emit signal
    emit newDataReady(data);

    if(hexFormatFlag)
    {
        dataStr = utilityBox.convertDataToHexString(data);
    }
    else
    {
        dataStr.append(data);
    }

    if(showRxPacketFlag)
    {
        updateLogData(dataStr);
    }
}

void SerialDebugWidget::autoSendData()
{
    on_pushButton_send_clicked();
}

void SerialDebugWidget::on_checkBox_autoSend_clicked(bool checked)
{
    if(checked)
    {
        autoSendTimer.start(ui->spinBox_autoSendInterval->value());
    }
    else
    {
        autoSendTimer.stop();
    }

    ui->spinBox_autoSendInterval->setEnabled(!checked);
}

void SerialDebugWidget::on_checkBox_timeStamp_clicked(bool checked)
{
    timeStampFlag = checked;
}

void SerialDebugWidget::on_checkBox_autoClear_clicked(bool checked)
{
    autoClearRxFlag = checked;
}

void SerialDebugWidget::on_checkBox_hex_clicked(bool checked)
{
    hexFormatFlag = checked;
}

void SerialDebugWidget::updateLogData(QString logStr)
{
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString("[yyyy-MM-dd hh:mm:ss:zzz] ");

    if(timeStampFlag)
    {
        // Add time stamp
        logStr.prepend(timeStr);
        logStr.append("\n");
    }

    if(autoClearRxFlag)
    {
        // Rx buffer > 20k bytes, clear and reset
        if(ui->textEdit_rxData->toPlainText().size() > 20000)
        {
            ui->textEdit_rxData->clear();
        }
    }

    ui->textEdit_rxData->insertPlainText(logStr); //Display in the textBrowser
    ui->textEdit_rxData->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
}

void SerialDebugWidget::updateConnectionStatus(struct COM_PORT_INIT_DATA *initData)
{
    if(NULL == initData || NULL == serialPort)
    {
        return;
    }

    updateComUI(initData);

    isOpenFlag = serialPort->isOpen();

    // Update UI display
    if(isOpenFlag)
    {
        ui->label_status->setStyleSheet(BG_COLOR_GREEN);
        ui->pushButton_open->setText(tr("Close"));
    }
    else
    {
        ui->label_status->setStyleSheet(BG_COLOR_RED);
        ui->pushButton_open->setText(tr("Open"));
    }

    setFunctionUI(!isOpenFlag);
}

void SerialDebugWidget::updateComUI(const struct COM_PORT_INIT_DATA *initData)
{
    memcpy(comInitData, initData, sizeof(struct COM_PORT_INIT_DATA));

    for(int i = 0; i < ui->comboBox_port->count(); i++)
    {
        if(QString(initData->port) == ui->comboBox_port->itemText(i))
        {
            ui->comboBox_port->setCurrentIndex(i);
            break;
        }
    }

    for(uint32_t i = 0; i < (sizeof(baudrate_combox) / sizeof(COMBOX_LIST)); i++)
    {
        if(initData->baudrate == baudrate_combox[i].value)
        {
            ui->comboBox_baudrate->setCurrentIndex(i);
            break;
        }
    }

    for(uint32_t i = 0; i < (sizeof(parity_combox) / sizeof(COMBOX_LIST)); i++)
    {
        if(initData->parity == parity_combox[i].value)
        {
            ui->comboBox_parity->setCurrentIndex(i);
            break;
        }
    }

    for(uint32_t i = 0; i < (sizeof(dataBits_combox) / sizeof(COMBOX_LIST)); i++)
    {
        if(initData->databits == dataBits_combox[i].value)
        {
            ui->comboBox_dataBits->setCurrentIndex(i);
            break;
        }
    }

    for(uint32_t i = 0; i < (sizeof(stopBits_combox) / sizeof(COMBOX_LIST)); i++)
    {
        if(initData->stopbits == stopBits_combox[i].value)
        {
            ui->comboBox_stopBits->setCurrentIndex(i);
            break;
        }
    }

    for(uint32_t i = 0; i < (sizeof(flowType_combox) / sizeof(COMBOX_LIST)); i++)
    {
        if(initData->flowtype == flowType_combox[i].value)
        {
            ui->comboBox_flowControl->setCurrentIndex(i);
            break;
        }
    }
}

void SerialDebugWidget::loadSettingFromIniFile()
{
    // Init default setting value
    initDefaultCOMSetting();

    QMutexLocker locker(&m_mutex);

    QString comStr = m_title;
    memset((char *)comInitData, 0, sizeof(struct COM_PORT_INIT_DATA));

    currentSetting->beginGroup(comStr.append("ComSetting"));

    QString portName = currentSetting->value("Port").toString();
    memcpy(comInitData->port, portName.toLatin1().data(), portName.toLatin1().length());

    switch(currentSetting->value("BaudRate").toInt())
    {
    case 9600:
        comInitData->baudrate = BAUD9600;
        break;
    case 19200:
        comInitData->baudrate = BAUD19200;
        break;
    case 38400:
        comInitData->baudrate = BAUD38400;
        break;
    case 57600:
        comInitData->baudrate = BAUD57600;
        break;
    case 115200:
        comInitData->baudrate = BAUD115200;
        break;
    default:
        comInitData->baudrate = BAUD115200;
        break;
    }

    switch(currentSetting->value("StopBits").toInt())
    {
    case 1:
        comInitData->stopbits = STOP_1;
        break;
    case 2:
        comInitData->stopbits = STOP_2;
        break;
    default:
        comInitData->stopbits = STOP_1;
        break;
    }

    switch(currentSetting->value("DataBits").toInt())
    {
    case 5:
        comInitData->databits = DATA_5;
        break;
    case 6:
        comInitData->databits = DATA_6;
        break;
    case 7:
        comInitData->databits = DATA_7;
        break;
    case 8:
        comInitData->databits = DATA_8;
        break;
    default:
        comInitData->databits = DATA_8;
        break;
    }

    if(currentSetting->value("Parity").toString() == "None")
    {
        comInitData->parity = PAR_NONE;
    }
    else if(currentSetting->value("Parity").toString() == "Odd")
    {
        comInitData->parity = PAR_ODD;
    }
    else if(currentSetting->value("Parity").toString() == "Even")
    {
        comInitData->parity = PAR_EVEN;
    }
    else
    {
        comInitData->parity = PAR_NONE;
    }

    if(currentSetting->value("FlowControl").toString() == "None")
    {
        comInitData->flowtype = FLOW_OFF;
    }
    else if(currentSetting->value("FlowControl").toString() == "Hardware")
    {
        comInitData->flowtype = FLOW_HARDWARE;
    }
    else if(currentSetting->value("FlowControl").toString() == "XonXoff")
    {
        comInitData->flowtype = FLOW_XONXOFF;
    }
    else
    {
        comInitData->flowtype = FLOW_OFF;
    }

    currentSetting->endGroup();

    // Update UIs
    updateComUI(comInitData);
}

void SerialDebugWidget::initDefaultCOMSetting()
{
    QMutexLocker locker(&m_mutex);

    QString comStr = m_title;
    currentSetting->beginGroup(comStr.append("ComSetting"));

    if(!currentSetting->contains("Port"))
    {
        // Init the default value
        currentSetting->setValue("Port", "COM1");
    }

    if(!currentSetting->contains("BaudRate"))
    {
        // Init the default value
        currentSetting->setValue("BaudRate", 115200);
    }

    if(!currentSetting->contains("StopBits"))
    {
        // Init the default value
        currentSetting->setValue("StopBits", 1);
    }

    if(!currentSetting->contains("DataBits"))
    {
        // Init the default value
        currentSetting->setValue("DataBits", 8);
    }

    if(!currentSetting->contains("Parity"))
    {
        // Init the default value
        currentSetting->setValue("Parity", "None");
    }

    if(!currentSetting->contains("FlowControl"))
    {
        // Init the default value
        currentSetting->setValue("FlowControl", "None");
    }

    currentSetting->endGroup();
}

void SerialDebugWidget::updateSettingToFile()
{
    QMutexLocker locker(&m_mutex);

    QString comStr = m_title;
    currentSetting->beginGroup(comStr.append("ComSetting"));

    currentSetting->setValue("Port", ui->comboBox_port->currentText());
    currentSetting->setValue("BaudRate", ui->comboBox_baudrate->currentText());
    currentSetting->setValue("StopBits", ui->comboBox_stopBits->currentText());
    currentSetting->setValue("DataBits", ui->comboBox_dataBits->currentText());
    currentSetting->setValue("Parity", ui->comboBox_parity->currentText());
    currentSetting->setValue("FlowControl", ui->comboBox_flowControl->currentText());

    currentSetting->endGroup();
}

void SerialDebugWidget::on_checkBox_showTx_clicked(bool checked)
{
    showTxPacketFlag = checked;
}

void SerialDebugWidget::on_checkBox_showRx_clicked(bool checked)
{
    showRxPacketFlag = checked;
}
