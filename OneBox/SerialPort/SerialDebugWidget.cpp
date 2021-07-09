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
    serialPort(portHandler),
    comInitData(new struct COM_PORT_INIT_DATA),
    widgetFontType("Arial"),
    widgetFontSize(10),
    isOpenFlag(false),
    timeStampFlag(false),
    hexFormatFlag(true),
    autoClearRxFlag(true)
{
    ui->setupUi(this);

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    connect(&refreshUITimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    refreshUITimer.start(1000);  //1s

    connect(&autoSendTimer, SIGNAL(timeout()), this, SLOT(autoSendData()));

    // Set Window Title
    this->setWindowTitle(tr("Serial Port Debug Widget"));
}

SerialDebugWidget::~SerialDebugWidget()
{
    delete ui;
    delete comInitData;
}

void SerialDebugWidget::resizeEvent(QResizeEvent *e)
{
    QWidget *pWidget = static_cast<QWidget*>(this->parent());

    if(pWidget != NULL)
    {
        this->resize(pWidget->size());
    }
}

void SerialDebugWidget::initWidgetFont()
{
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
}

void SerialDebugWidget::initWidgetStyle()
{
    ui->spinBox_autoSendInterval->setRange(1, 1000000);
    ui->spinBox_autoSendInterval->setValue(1000);

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

        QStringList comPorts = serialPort->getAvailablePorts();
        ui->comboBox_port->clear();
        ui->comboBox_port->insertItems(0, comPorts);
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

    updateLogData(dataStr);
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
