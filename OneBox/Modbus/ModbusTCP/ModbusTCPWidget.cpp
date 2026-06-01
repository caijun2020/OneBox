/**********************************************************************
PACKAGE:        UI
FILE:           ModbusTCPWidget.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Modbus TCP UI widget
**********************************************************************/

#include "ModbusTCPWidget.h"
#include "ui_ModbusTCPWidget.h"
#include <QDateTime>
#include <QMessageBox>

#include "QUtilityBox.h"
#include "QtBaseType.h"

//#define MODBUSTCP_DEBUG_PRINT

static ModbusTCPWidget::COMBOX_LIST functionCode_combox[] =
{
    {(int)MB_FUNC_READ_HOLDING_REGISTER, "Read Holding Reg"},
    {(int)MB_FUNC_READ_INPUT_REGISTER, "Read Input Reg"},
    {(int)MB_FUNC_WRITE_MULTIPLE_REGISTERS, "Write Multi Reg"},
};

ModbusTCPWidget::ModbusTCPWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModbusTCPWidget),
    refreshTimer(new QTimer),
    refreshInMs(1000),
    m_settingFile("config.ini"),
    m_modbusTCP(NULL),
    m_transactionID(0x0000),
    m_protocolID(0x0000),
    m_unitID(1),
    intervalTimeInMs(0),
    intervalTime(new QTime),
    periodTxTimeInMs(100),
    TX_RETRY_MAX_TIMES(1),
    isRunning(false),
    hexFormatFlag(false),
    autoClearRxFlag(true),
    m_serverIP("127.0.0.1"),
    m_serverPort(502),
    m_autoConnectToServerFlag(true),
    showTxRxFlag(false)
{
    ui->setupUi(this);

    // Prepend the exe absolute path
    m_settingFile.prepend(QUtilityBox::instance()->getAppDirPath());

    // Default setting file
    currentSetting = new QSettings(m_settingFile, QSettings::IniFormat);

    // Load Settings from ini file
    loadSettingFromIniFile();

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    refreshTimer->start(refreshInMs);  //1s

    // Set Window Title
    this->setWindowTitle(tr("ModbusTCP Communication"));
}

ModbusTCPWidget::~ModbusTCPWidget()
{
    delete ui;
    delete refreshTimer;

    delete currentSetting;
    delete intervalTime;
}

void ModbusTCPWidget::bindModel(ModbusTCP *clientP)
{
    if(NULL != clientP)
    {
        unbind();

        m_modbusTCP = clientP;

        m_modbusTCP->setTransactionID(m_transactionID);
        m_modbusTCP->setProtocolID(m_protocolID);
        m_modbusTCP->setUnitID(m_unitID);
        m_modbusTCP->setTxPeriod(periodTxTimeInMs);
        m_modbusTCP->setTxRetryTimes(TX_RETRY_MAX_TIMES);
        m_modbusTCP->setAutoReconnect(m_autoConnectToServerFlag);

        connect(m_modbusTCP, SIGNAL(connectionChanged(bool)), this, SLOT(updateConnectionStatus(bool)));
        connect(m_modbusTCP, SIGNAL(newDataReady(QByteArray)), this, SLOT(readDataFromModbus(QByteArray)));
        connect(m_modbusTCP, SIGNAL(newDataTx(QByteArray)), this, SLOT(updateTxDataToLog(QByteArray)));

        isRunning = m_modbusTCP->getRunningStatus();

        // If client is not running, start it
        if(!isRunning)
        {
            // Enable Listen with delay
            QTimer::singleShot(refreshInMs, this, SLOT(on_pushButton_connect_clicked()));
        }
    }
}

void ModbusTCPWidget::unbind()
{
    if(NULL != m_modbusTCP)
    {
        disconnect(m_modbusTCP, 0 , this , 0);
    }

    m_modbusTCP = NULL;
}

void ModbusTCPWidget::setUnitID(uint8_t id)
{
    m_unitID = id;
    ui->lineEdit_unitID->setText(QString::number(m_unitID));

    if(NULL != m_modbusTCP)
    {
        m_modbusTCP->setUnitID(m_unitID);
    }
}

void ModbusTCPWidget::updateUI()
{
}

void ModbusTCPWidget::retranslateUI()
{
    ui->retranslateUi(this);
}

void ModbusTCPWidget::loadSettingFromIniFile()
{
    QMutexLocker locker(&m_mutex);

    // Load Modbus setting
    currentSetting->beginGroup("ModbusTcpSetting");

    if(currentSetting->contains("ServerIP"))
    {
        // Load Server IP
        m_serverIP = currentSetting->value("ServerIP").toString();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ServerIP", m_serverIP);
    }
    ui->lineEdit_serverIP->setText(m_serverIP);

    if(currentSetting->contains("Port"))
    {
        // Load Server Port
        m_serverPort = currentSetting->value("Port").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("Port", m_serverPort);
    }
    ui->lineEdit_serverPort->setText(QString::number(m_serverPort));

    if(currentSetting->contains("TransactionID"))
    {
        // Load Transaction ID
        m_transactionID = currentSetting->value("TransactionID").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("TransactionID", m_transactionID);
    }
    ui->lineEdit_transactionID->setText(QString::number(m_transactionID));

    if(currentSetting->contains("ResponseTime"))
    {
        // Load Period Tx Max time
        periodTxTimeInMs = currentSetting->value("ResponseTime").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ResponseTime", periodTxTimeInMs);
    }

    if(currentSetting->contains("ResponseRetry"))
    {
        // Load Tx Retry count
        TX_RETRY_MAX_TIMES = currentSetting->value("ResponseRetry").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("ResponseRetry", TX_RETRY_MAX_TIMES);
    }

    if(currentSetting->contains("UnitID"))
    {
        // Load Unit ID
        m_unitID = currentSetting->value("UnitID").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("UnitID", m_unitID);
    }
    ui->lineEdit_unitID->setText(QString::number(m_unitID));

    if(currentSetting->contains("AutoReconnect"))
    {
        // Load Auto reconnect flag
        m_autoConnectToServerFlag = currentSetting->value("AutoReconnect").toBool();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("AutoReconnect", m_autoConnectToServerFlag);
    }

    currentSetting->endGroup();
}

void ModbusTCPWidget::updateSettingToFile()
{
    QMutexLocker locker(&m_mutex);

    currentSetting->beginGroup("ModbusTcpSetting");
    currentSetting->setValue("ServerIP", m_serverIP);
    currentSetting->setValue("Port", m_serverPort);
    currentSetting->setValue("TransactionID", m_transactionID);
    currentSetting->setValue("UnitID", m_unitID);
    currentSetting->setValue("AutoReconnect", m_autoConnectToServerFlag);
    currentSetting->endGroup();
}

void ModbusTCPWidget::initWidgetFont()
{
}

void ModbusTCPWidget::initWidgetStyle()
{
    for(uint32_t i = 0; i < (sizeof(functionCode_combox) / sizeof(COMBOX_LIST)); i++)
    {
        ui->comboBox_functionCode->insertItem(i, functionCode_combox[i].text);
    }

    ui->checkBox_autoClear->setChecked(autoClearRxFlag);
    ui->checkBox_hex->setChecked(hexFormatFlag);

    ui->lineEdit_transactionID->setText(QString::number(m_transactionID));
    ui->lineEdit_protocolID->setText(QString::number(m_protocolID));
    ui->lineEdit_protocolID->setReadOnly(true);

    ui->label_status->setText("");

    // Set port range from 0 to 65535
    ui->lineEdit_serverPort->setValidator(new QIntValidator(0, 65535, this));

    // Set transaction ID range from 0 to 65535
    ui->lineEdit_transactionID->setValidator(new QIntValidator(0, 65535, this));

    // Set unit ID range from 0 to 255
    ui->lineEdit_unitID->setValidator(new QIntValidator(0, 255, this));

    ui->checkBox_showTxRx->setChecked(showTxRxFlag);
    ui->checkBox_autoReconnect->setChecked(m_autoConnectToServerFlag);
}

void ModbusTCPWidget::on_pushButton_connect_clicked()
{
    QString logStr;

    if(NULL == m_modbusTCP)
    {
        return;
    }

    // Toggle flag
    isRunning = !isRunning;

    if(true == isRunning)
    {
        // Connect to server
        isRunning = m_modbusTCP->connectToServer(ui->lineEdit_serverIP->text(), ui->lineEdit_serverPort->text().toInt());
        logStr = tr("Connect to %1:%2 ").arg(ui->lineEdit_serverIP->text())
                      .arg(ui->lineEdit_serverPort->text());

        if(isRunning)
        {
            logStr.append(tr("succeed"));

            m_modbusTCP->startPeriodTxService();
        }
        else
        {
            logStr.append(tr("failed"));
        }

        // Update log
        updateToUILog(logStr);
    }
    else
    {
        // Disconnect from server
        m_modbusTCP->disconnectFromServer();
    }
}

bool ModbusTCPWidget::getConnectionStatus() const
{
    return isRunning;
}

void ModbusTCPWidget::autoConnectToServer()
{
    if(m_autoConnectToServerFlag)
    {
        if(!isRunning)
        {
            on_pushButton_connect_clicked();
        }
    }
}

void ModbusTCPWidget::on_checkBox_hex_clicked(bool checked)
{
    hexFormatFlag = checked;
}

void ModbusTCPWidget::on_checkBox_autoClear_clicked(bool checked)
{
    autoClearRxFlag = checked;
}

void ModbusTCPWidget::on_pushButton_clear_clicked()
{
    ui->textEdit_log->clear();
}

void ModbusTCPWidget::on_pushButton_send_clicked()
{
    QUtilityBox utilityBox;
    uint32_t txLen = 0;
    uint16_t tempBuf[TX_BUF_SIZE] = {0};
    uint32_t addrOffset = ui->lineEdit_regAddr->text().toInt();
    uint32_t regCnt = ui->lineEdit_regCount->text().toInt();

    uint8_t functionCode = functionCode_combox[ui->comboBox_functionCode->currentIndex()].value;

    switch(functionCode)
    {
    case (int)MB_FUNC_READ_HOLDING_REGISTER:
        m_modbusTCP->readHoldRegisters(addrOffset, regCnt);
        break;
    case (int)MB_FUNC_READ_INPUT_REGISTER:
        m_modbusTCP->readInputRegisters(addrOffset, regCnt);
        break;
    case (int)MB_FUNC_WRITE_MULTIPLE_REGISTERS:

        // Hex format
        if(hexFormatFlag)
        {
            txLen = utilityBox.convertHexStringToDataBuffer(tempBuf, ui->lineEdit_data->text());
        }
        else
        {
            txLen = utilityBox.convertDecStringToDataBuffer(tempBuf, ui->lineEdit_data->text());
        }

        if(txLen != regCnt)
        {
            //@TODO
        }

        m_modbusTCP->writeMultiRegisters(addrOffset, (char *)tempBuf, regCnt);
        break;
    default:
        break;
    }
}

void ModbusTCPWidget::on_lineEdit_serverIP_editingFinished()
{
    m_serverIP = ui->lineEdit_serverIP->text();

    // Update to ini setting
    updateSettingToFile();
}

void ModbusTCPWidget::on_lineEdit_serverPort_editingFinished()
{
    m_serverPort = ui->lineEdit_serverPort->text().toInt();

    // Update to ini setting
    updateSettingToFile();
}

void ModbusTCPWidget::on_comboBox_functionCode_currentIndexChanged(int index)
{
    ui->lineEdit_functionCode->setText(QString::number(functionCode_combox[index].value));
}

void ModbusTCPWidget::on_lineEdit_transactionID_editingFinished()
{
    m_transactionID = ui->lineEdit_transactionID->text().toInt();

    // Update to ini setting
    updateSettingToFile();

    if(NULL != m_modbusTCP)
    {
        m_modbusTCP->setTransactionID(m_transactionID);
    }
}

void ModbusTCPWidget::on_lineEdit_protocolID_editingFinished()
{
    m_protocolID = ui->lineEdit_protocolID->text().toInt();

    if(NULL != m_modbusTCP)
    {
        m_modbusTCP->setProtocolID(m_protocolID);
    }
}

void ModbusTCPWidget::on_lineEdit_unitID_editingFinished()
{
    m_unitID = ui->lineEdit_unitID->text().toInt();

    // Update to ini setting
    updateSettingToFile();

    if(NULL != m_modbusTCP)
    {
        m_modbusTCP->setUnitID(m_unitID);
    }
}

void ModbusTCPWidget::on_lineEdit_data_textChanged(const QString &arg1)
{
    QStringList dataList;
    QString inputStr = arg1;

    dataList = inputStr.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    ui->lineEdit_regCount->setText(QString::number(dataList.size()));
}

void ModbusTCPWidget::updateLogData(QString logStr)
{
    Q_UNUSED(logStr);
}

void ModbusTCPWidget::updateToUILog(QString logStr, bool timeStamp)
{
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString("[yyyy-MM-dd hh:mm:ss.zzz] ");

    if(timeStamp)
    {
        // Add time stamp
        logStr.prepend(timeStr);
    }

    if(autoClearRxFlag)
    {
        // Rx buffer > 20k bytes, clear and reset
        if(ui->textEdit_log->toPlainText().size() > 20000)
        {
            ui->textEdit_log->clear();
        }
    }

    logStr.append("\n");
    ui->textEdit_log->insertPlainText(logStr); //Display the data in the textBrowse
    ui->textEdit_log->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
}

void ModbusTCPWidget::updateConnectionStatus(bool connected)
{
    QString logStr;
    isRunning = connected;

    if(isRunning)
    {
        ui->pushButton_connect->setText(tr("Disconnect"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_GREEN);

        logStr = tr("Connect to %1:%2 ").arg(ui->lineEdit_serverIP->text())
                      .arg(ui->lineEdit_serverPort->text());
        logStr.append(tr("succeed"));

        m_modbusTCP->startPeriodTxService();
    }
    else
    {
        ui->pushButton_connect->setText(tr("Connect"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_RED);

        // Show warning dialog
        static int cnt = 0;
        logStr = tr("Disconnect from ModbusTcp Server %1:%2")
                                .arg(ui->lineEdit_serverIP->text())
                                .arg(ui->lineEdit_serverPort->text());

        if(1 == ++cnt)
        {
            //QMessageBox::warning(this, tr("Warning"), logStr);
        }
        else
        {
        }
    }

    // Update log
    updateToUILog(logStr);
}

void ModbusTCPWidget::readDataFromModbus(QByteArray data)
{
    QString logStr;
    logStr.clear();

    intervalTimeInMs = intervalTime->elapsed();

    if(showTxRxFlag)
    {
        QString intervalStr = "(";
        intervalStr.append(QString::number(intervalTimeInMs));
        intervalStr.append(" ms)");

        //Display the data in the textBrowse
        updateToUILog(intervalStr, false);

        logStr.append(tr("Rx Data:"));
        for(int i = 0; i < data.size(); i++)
        {
            logStr.append(QString::number((uint8_t)data.at(i), 16).rightJustified(2, '0').toUpper());
            logStr.append(" ");
        }

        // Update log
        updateToUILog(logStr);
    }
}

void ModbusTCPWidget::on_checkBox_showTxRx_clicked(bool checked)
{
    showTxRxFlag = checked;
}

void ModbusTCPWidget::updateTxDataToLog(QByteArray data)
{
    QString logStr;
    logStr.clear();

    if(showTxRxFlag)
    {
        logStr.append(tr("Tx Data:"));
        for(int i = 0; i < data.size(); i++)
        {
            logStr.append(QString::number((uint8_t)data.at(i), 16).rightJustified(2, '0').toUpper());
            logStr.append(" ");
        }

        // Update log
        updateToUILog(logStr);
    }

    intervalTime->restart();
}

void ModbusTCPWidget::on_checkBox_autoReconnect_clicked(bool checked)
{
    m_autoConnectToServerFlag = checked;

    if(NULL != m_modbusTCP)
    {
        m_modbusTCP->setAutoReconnect(m_autoConnectToServerFlag);
    }

    // Update to ini setting
    updateSettingToFile();
}
