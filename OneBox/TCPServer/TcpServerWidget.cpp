#include "TcpServerWidget.h"
#include "ui_TcpServerWidget.h"
#include <QNetworkInterface>
#include <QDateTime>
#include "QtBaseType.h"
#include "QUtilityBox.h"


TcpServerWidget::TcpServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpServerWidget),
    tcpServer(NULL),
    isRunning(false),
    hexFormatFlag(false),
    autoClearRxFlag(true),
    serverIP("192.168.2.102"),
    listenPort(50000)
{
    ui->setupUi(this);

    // Default setting file
    currentSetting = new QSettings("config.ini", QSettings::IniFormat);

    // Load Settings from ini file
    loadSettingFromIniFile();

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    connect(&refreshUITimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    refreshUITimer.start(1000);  //1s

    // Set Window Title
    this->setWindowTitle( tr("Tcp Server Widget") );
}

TcpServerWidget::~TcpServerWidget()
{
    delete ui;
    delete currentSetting;
}

void TcpServerWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    QWidget *pWidget = static_cast<QWidget*>(this->parent());

    if(pWidget != NULL)
    {
        this->resize(pWidget->size());
    }
}

void TcpServerWidget::bindModel(TCPServer *serverP)
{
    if(NULL != serverP)
    {
        unbind();

        tcpServer = serverP;

        connect(tcpServer, SIGNAL(connectionIn(QString)), this, SLOT(addIncomingClient(QString)));
        connect(tcpServer, SIGNAL(connectionOut(QString)), this, SLOT(removeIncomingClient(QString)));
        connect(tcpServer, SIGNAL(newDataReady(uint32_t)), this, SLOT(updateIncomingData(uint32_t)));
        connect(tcpServer, SIGNAL(newDataTx(QHostAddress,uint16_t,QByteArray)), this, SLOT(updateTxDataToLog(QHostAddress,uint16_t,QByteArray)));

        connect(tcpServer, SIGNAL(serverChanged(QHostAddress,uint16_t)), this, SLOT(updateServerInfo(QHostAddress,uint16_t)));
        connect(tcpServer, SIGNAL(connectionChanged(bool)), this, SLOT(updateConnectionStatus(bool)));

        isRunning = tcpServer->getRunningStatus();
        updateConnectionStatus(isRunning);

        // If server is not running, start listen
        if(!isRunning)
        {
            // Enable Listen
            on_pushButton_listen_clicked();
        }
    }
}

void TcpServerWidget::unbind()
{
    if(NULL != tcpServer)
    {
        disconnect(tcpServer, 0 , this , 0);
    }

    tcpServer = NULL;
}

void TcpServerWidget::initWidgetFont()
{
}

void TcpServerWidget::initWidgetStyle()
{
    // Update Status Color
    ui->label_status->setText("");
    updateConnectionStatus(isRunning);

    // Get host IP address
    //ui->lineEdit_IP->setText(QNetworkInterface().allAddresses().at(1).toString());
    qDebug() << "All IP Address: " << QNetworkInterface().allAddresses();

    ui->checkBox_hex->setChecked(hexFormatFlag);
    ui->checkBox_autoClear->setChecked(autoClearRxFlag);
}

void TcpServerWidget::loadSettingFromIniFile()
{
    currentSetting->beginGroup("TCPServer");

    if(currentSetting->contains("IP"))
    {
        // Load IP
        serverIP = currentSetting->value("IP").toString();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("IP", serverIP);
    }
    ui->lineEdit_IP->setText(serverIP);

    if(currentSetting->contains("Port"))
    {
        // Load listen port
        listenPort = currentSetting->value("Port").toInt();
    }
    else
    {
        // Init the default value
        currentSetting->setValue("Port", listenPort);
    }
    ui->lineEdit_listenPort->setText(QString::number(listenPort));

    currentSetting->endGroup();
}

void TcpServerWidget::on_pushButton_listen_clicked()
{
    if(NULL == tcpServer)
    {
        return;
    }

    // Toggle flag
    isRunning = !isRunning;

    if(true == isRunning)
    {
        tcpServer->beginListen(QHostAddress::Any, ui->lineEdit_listenPort->text().toInt());
    }
    else
    {
        tcpServer->stopListen();
    }
}

void TcpServerWidget::on_pushButton_send_clicked()
{
    uint32_t txLen = 0;
    uint8_t tempBuf[10000] = {0};
    QByteArray tempTxBuf;
    QUtilityBox utilityBox;

    // If there is no incoming client connected
    if(ui->comboBox_clients->count() <= 1)
    {
        return;
    }

    // Hex format
    if(hexFormatFlag)
    {
        txLen = utilityBox.convertHexStringToDataBuffer(tempBuf, ui->textEdit_txData->toPlainText());
        tempTxBuf = QByteArray((char *)tempBuf, txLen);
    }
    else
    {
        tempTxBuf = ui->textEdit_txData->toPlainText().toLatin1();
    }

    // Send msg to all
    if((ui->comboBox_clients->count() - 1) == ui->comboBox_clients->currentIndex())
    {
        for(uint32_t i = 0; i < tcpServer->getConnectionCount(); i++)
        {
            sendData(i, tempTxBuf);
        }
    }
    else
    {
        sendData(ui->comboBox_clients->currentIndex(), tempTxBuf);
    }
}

void TcpServerWidget::on_pushButton_clear_clicked()
{
    ui->textEdit_log->clear();
}

void TcpServerWidget::addIncomingClient(QString str)
{
    QString logStr;

    logStr.append(tr("New client connected:"));
    logStr.append(str);

    // Update log
    updateLogData(logStr);

    ui->comboBox_clients->insertItem(ui->comboBox_clients->count() - 1, str);
}

void TcpServerWidget::removeIncomingClient(QString str)
{
    QString logStr;

    logStr.append(tr("Client removed:"));
    logStr.append(str);

    // Update log
    updateLogData(logStr);

    ui->comboBox_clients->removeItem(ui->comboBox_clients->findText(str));
}

void TcpServerWidget::updateIncomingData(uint32_t clientIndex)
{
    QString logStr;

    rxDataBuf.clear();
    if(tcpServer->getUndealData(rxDataBuf))
    {
        logStr.append(tr("Rx data from %1").arg(ui->comboBox_clients->itemText(clientIndex)));
        // Update log
        updateLogData(logStr);

        logStr.clear();
        logStr.append(tr("Rx Data:"));
        for(int i = 0; i < rxDataBuf.size(); i++)
        {
            logStr.append(QString::number((uint8_t)rxDataBuf.at(i), 16).rightJustified(2, '0').toUpper());
            logStr.append(" ");
        }
        logStr.append("(");
        logStr.append(rxDataBuf);
        logStr.append(")");
        // Update log
        updateLogData(logStr);

        // Emit signal, new comming data received
        emit newDataReady(clientIndex, rxDataBuf);
    }
}

void TcpServerWidget::sendData(uint32_t clientIndex, QByteArray &data)
{
    QString logStr;

    if(NULL == tcpServer)
    {
        return;
    }

    if(clientIndex >= tcpServer->getConnectionCount())
    {
        logStr = tr("clientIndex is out of connections");
        // Update log
        updateLogData(logStr);
        qDebug() << logStr;

        return;
    }

    tcpServer->sendData(clientIndex, data);
}

void TcpServerWidget::sendData(uint32_t clientIndex, const char *data, uint32_t len)
{
    QByteArray temp(data, len);

    sendData(clientIndex, temp);
}

void TcpServerWidget::updateLogData(QString logStr)
{
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString("[yyyy-MM-dd hh:mm:ss:zzz] ");

    // Add time stamp
    logStr.prepend(timeStr);

    if(autoClearRxFlag)
    {
        // Rx buffer > 200k bytes, clear and reset
        if(ui->textEdit_log->toPlainText().size() > 200000)
        {
            ui->textEdit_log->clear();
        }
    }

    //logFile->addLogToFile(logStr);
    ui->textEdit_log->insertPlainText(logStr.append("\n")); //Display the log in the textBrowse
    ui->textEdit_log->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
}

void TcpServerWidget::updateTxDataToLog(QHostAddress address, uint16_t port, QByteArray data)
{
    QString logStr;

    logStr.append(tr("Send data to %1:%2")
                  .arg(address.toString())
                  .arg(port));
    // Update log
    updateLogData(logStr);

    logStr.clear();
    logStr.append(tr("Tx Data:"));
    for(int i = 0; i < data.size(); i++)
    {
        logStr.append(QString::number((uint8_t)data.at(i), 16).rightJustified(2, '0').toUpper());
        logStr.append(" ");
    }
    logStr.append("(");
    logStr.append(data);
    logStr.append(")");
    // Update log
    updateLogData(logStr);
}

void TcpServerWidget::updateUI()
{
    static uint32_t lastRxPacketCnt = 0;
    static uint32_t lastTxPacketCnt = 0;

    if(NULL == tcpServer)
    {
        return;
    }

    ui->label_rxPacketCnt->setText(QString::number(tcpServer->getRxDiagramCnt()));
    ui->label_txPacketCnt->setText(QString::number(tcpServer->getTxDiagramCnt()));
    ui->label_rxPacketCntAvg->setText(QString::number(tcpServer->getRxDiagramCnt() - lastRxPacketCnt));
    ui->label_txPacketCntAvg->setText(QString::number(tcpServer->getTxDiagramCnt() - lastTxPacketCnt));
    ui->label_rxBytesCnt->setText(QString::number(tcpServer->getTotalRxBytes()));
    ui->label_txBytesCnt->setText(QString::number(tcpServer->getTotalTxBytes()));

    lastRxPacketCnt = tcpServer->getRxDiagramCnt();
    lastTxPacketCnt = tcpServer->getTxDiagramCnt();
}

void TcpServerWidget::on_pushButton_reset_clicked()
{
    if(NULL == tcpServer)
    {
        return;
    }

    tcpServer->resetTxRxCnt();
}

void TcpServerWidget::on_checkBox_hex_clicked(bool checked)
{
    hexFormatFlag = checked;
}

void TcpServerWidget::on_checkBox_autoClear_clicked(bool checked)
{
    autoClearRxFlag = checked;
}

void TcpServerWidget::updateServerInfo(QHostAddress address, uint16_t port)
{
    ui->lineEdit_IP->setText(address.toString());
    ui->lineEdit_listenPort->setText(QString::number(port));
}

void TcpServerWidget::updateConnectionStatus(bool connected)
{
    isRunning = connected;

    if(true == isRunning)
    {
        ui->pushButton_listen->setText(tr("Stop Listen"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_GREEN);
    }
    else
    {
        ui->pushButton_listen->setText(tr("Start Listen"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_RED);
    }
}

void TcpServerWidget::updateSettingToFile()
{
    currentSetting->beginGroup("TCPServer");
    currentSetting->setValue("IP", serverIP);
    currentSetting->setValue("Port", listenPort);
    currentSetting->endGroup();
}

void TcpServerWidget::on_lineEdit_IP_editingFinished()
{
    serverIP = ui->lineEdit_IP->text();

    // Update setting to ini file
    updateSettingToFile();
}

void TcpServerWidget::on_lineEdit_listenPort_editingFinished()
{
    listenPort = ui->lineEdit_listenPort->text().toInt();

    // Update setting to ini file
    updateSettingToFile();
}
