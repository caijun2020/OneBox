#include "ModbusRTUWidget.h"
#include "ui_ModbusRTUWidget.h"

#include <QMessageBox>
#include <QMutexLocker>
#include <QDebug>
#include "QUtilityBox.h"

//#define MODBUSRTU_DEBUG_PRINT


static ModbusRTUWidget::COMBOX_LIST functionCode_combox[] =
{
    {(int)MB_FUNC_READ_HOLDING_REGISTER, "Read Holding Reg"},
    {(int)MB_FUNC_WRITE_MULTIPLE_REGISTERS, "Write Multi Reg"},
};

ModbusRTUWidget::ModbusRTUWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModbusRTUWidget),
    refreshTimer(new QTimer),
    refreshInMs(1000),
    m_settingFile("config.ini"),
    m_modbusRTU(NULL),
    widgetFontType("Arial"),
    widgetFontSize(16),
    intervalTimeInMs(0),
    intervalTime(new QTime),
    logFile(new FileLog),
    logPath("./Log/"),
    txBufLen(0),
    hexFormatFlag(false),
    autoClearRxFlag(true)
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
    this->setWindowTitle(tr("ModbusRTU Communication"));
}

ModbusRTUWidget::~ModbusRTUWidget()
{
    qDebug() << "~ModbusRTUWidget()";
    delete ui;
    delete refreshTimer;

    delete currentSetting;
    delete logFile;

    delete intervalTime;
}

void ModbusRTUWidget::bindModel(ModbusRTU *modbusRTUP)
{
    if(NULL != modbusRTUP)
    {
        unbind();

        m_modbusRTU = modbusRTUP;
        connect(m_modbusRTU, SIGNAL(newDataReady(QByteArray)), this, SLOT(readDataFromModbus(QByteArray)));
        connect(m_modbusRTU, SIGNAL(newDataTx(QByteArray)), this, SLOT(updateTxDataToLog(QByteArray)));
        connect(m_modbusRTU, SIGNAL(reportModbusResponseValue(MODBUS_READ_FEEDBACK)), this, SLOT(handleModbusResponseValue(MODBUS_READ_FEEDBACK)));

        setSlaveAddr(m_modbusRTU->getSlaveAddr());
    }
}

void ModbusRTUWidget::unbind()
{
    if(NULL != m_modbusRTU)
    {
        disconnect(m_modbusRTU, 0 , this , 0);
    }

    m_modbusRTU = NULL;
}

void ModbusRTUWidget::updateUI()
{
}

void ModbusRTUWidget::retranslateUI()
{
    ui->retranslateUi(this);
}

void ModbusRTUWidget::loadSettingFromIniFile()
{
    // Load Font type and size
    currentSetting->beginGroup("SystemSetting");

    if(currentSetting->contains("FontType"))
    {
        widgetFontType = currentSetting->value("FontType").toString();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("FontType", widgetFontType);
    }

    if(currentSetting->contains("FontSize"))
    {
        widgetFontSize = currentSetting->value("FontSize").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("FontSize", widgetFontSize);
    }

    if(currentSetting->contains("LogPath"))
    {
        // Load Log Path
        logPath = currentSetting->value("LogPath").toString();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("LogPath", logPath);
    }
    // Init Log File Path
    logFile->setLogPath(logPath);

    currentSetting->endGroup();


    // Load Modbus setting
    currentSetting->beginGroup("ModbusSetting");
    currentSetting->endGroup();

}

void ModbusRTUWidget::initWidgetFont()
{
}

void ModbusRTUWidget::initWidgetStyle()
{
    for(uint32_t i = 0; i < (sizeof(functionCode_combox) / sizeof(COMBOX_LIST)); i++)
    {
        ui->comboBox_functionCode->insertItem(i, functionCode_combox[i].text);
    }

    ui->checkBox_autoClear->setChecked(autoClearRxFlag);
    ui->checkBox_hex->setChecked(hexFormatFlag);

    // Set address range from 0 to 255
    ui->lineEdit_address->setValidator(new QIntValidator(0, 255, this));
}

void ModbusRTUWidget::setSlaveAddr(uint8_t addr)
{
    m_devAddr = addr;
    ui->lineEdit_address->setText(QString::number(m_devAddr));
}

uint8_t ModbusRTUWidget::getSlaveAddr() const
{
    return m_devAddr;
}

bool ModbusRTUWidget::getModbusCommOk()
{
    bool ret = false;

    if(NULL != m_modbusRTU)
    {
        ret = m_modbusRTU->getModbusCommOk();
    }

    return ret;
}

void ModbusRTUWidget::updateLogData(QString logStr)
{
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString("[yyyy-MM-dd hh:mm:ss.zzz] ");

    // Add time stamp
    logStr.prepend(timeStr);

    logFile->addLogToFile(logStr);
    //ui->textEdit_log->insertPlainText(logStr.append("\n")); //Display the log in the textBrowse
    //ui->textEdit_log->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
}

void ModbusRTUWidget::updateToUILog(QString logStr, bool timeStamp)
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

void ModbusRTUWidget::on_pushButton_clear_clicked()
{
    ui->textEdit_log->clear();
}

void ModbusRTUWidget::on_pushButton_send_clicked()
{
    QUtilityBox utilityBox;
    uint32_t txLen = 0;
    uint16_t tempBuf[TX_BUF_SIZE] = {0};
    uint32_t addrOffset = ui->lineEdit_regAddr->text().toInt();
    uint32_t regCnt = ui->lineEdit_regCount->text().toInt();
    uint8_t functionCode = functionCode_combox[ui->comboBox_functionCode->currentIndex()].value;

    if(NULL == m_modbusRTU)
    {
        return;
    }

    switch(functionCode)
    {
    case (int)MB_FUNC_READ_HOLDING_REGISTER:
        m_modbusRTU->readHoldRegisters(addrOffset, regCnt);
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

        m_modbusRTU->writeMultiRegisters(addrOffset, (char *)tempBuf, regCnt);
        break;
    default:
        break;
    }
}

void ModbusRTUWidget::on_checkBox_autoClear_clicked(bool checked)
{
    autoClearRxFlag = checked;
}

void ModbusRTUWidget::on_checkBox_hex_clicked(bool checked)
{
    hexFormatFlag = checked;
}

void ModbusRTUWidget::on_comboBox_functionCode_currentIndexChanged(int index)
{
    ui->lineEdit_functionCode->setText(QString::number(functionCode_combox[index].value));
}

void ModbusRTUWidget::on_lineEdit_data_textEdited(const QString &arg1)
{
    QStringList dataList;
    QString inputStr = arg1;

    dataList = inputStr.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    ui->lineEdit_regCount->setText(QString::number(dataList.size()));
}

void ModbusRTUWidget::on_lineEdit_address_editingFinished()
{
    m_devAddr = ui->lineEdit_address->text().toInt();

    if(NULL != m_modbusRTU)
    {
        m_modbusRTU->setSlaveAddr(m_devAddr);
    }

    // Update to ini setting
    currentSetting->setValue("ModbusSetting/SlaveAddress", m_devAddr);
}

void ModbusRTUWidget::readDataFromModbus(QByteArray data)
{
    QString logStr;
    logStr.clear();

    intervalTimeInMs = intervalTime->elapsed();

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

void ModbusRTUWidget::updateTxDataToLog(QByteArray data)
{
    QString logStr;
    logStr.clear();

    logStr.append(tr("Tx Data:"));
    for(int i = 0; i < data.size(); i++)
    {
        logStr.append(QString::number((uint8_t)data.at(i), 16).rightJustified(2, '0').toUpper());
        logStr.append(" ");
    }

    // Update log
    updateToUILog(logStr);

    intervalTime->restart();
}

void ModbusRTUWidget::handleModbusResponseValue(MODBUS_READ_FEEDBACK data)
{
    QString logStr;
    logStr.clear();

    logStr.append(tr("Modbus Response:\n"));
    logStr.append(tr("Address:%1\n").arg(data.address));
    logStr.append(tr("Data:"));
    for(int i = 0; i < data.len; i++)
    {
        logStr.append(QString::number((uint8_t)data.buffer[2*i], 16).rightJustified(2, '0').toUpper());
        logStr.append(" ");
        logStr.append(QString::number((uint8_t)data.buffer[2*i + 1], 16).rightJustified(2, '0').toUpper());
        logStr.append(" ");
    }

    // Update log
    //updateToUILog(logStr);
}
