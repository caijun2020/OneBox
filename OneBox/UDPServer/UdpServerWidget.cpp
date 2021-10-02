/**********************************************************************
PACKAGE:        Communication
FILE:           UdpServerWidget.cpp
COPYRIGHT (C):  All rights reserved.

PURPOSE:        UDP Server Widget UI
**********************************************************************/

#include "UdpServerWidget.h"
#include "ui_UdpServerWidget.h"

#include <QDateTime>
#include <QNetworkInterface>
#include <QDebug>
#include "QtBaseType.h"
#include "QUtilityBox.h"

UdpServerWidget::UdpServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UdpServerWidget),
    udpServer(NULL),
    isServerRunning(false),
    hexFormatFlag(false),
    autoClearRxFlag(true)
{
    ui->setupUi(this);

    // Init Widget Font type and size
    initWidgetFont();

    // Init Widget Style
    initWidgetStyle();

    // Reserve 4Kbytes
    rxDataBuf.reserve(4000);

    // Init signal & slot
    connect(&refreshUITimer, SIGNAL(timeout()), this, SLOT(updateUI()));
    refreshUITimer.start(1000);  //1s

    // Set Window Title
    this->setWindowTitle(tr("UDP Server Widget"));
}

UdpServerWidget::~UdpServerWidget()
{
    delete ui;
}

void UdpServerWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    QWidget *pWidget = static_cast<QWidget*>(this->parent());

    if(pWidget != NULL)
    {
        this->resize(pWidget->size());
    }
}

void UdpServerWidget::bindModel(UDPServer *serverP)
{
    if(NULL != serverP)
    {
        unbind();

        udpServer = serverP;

        connect(udpServer, SIGNAL(connectionIn(QString)), this, SLOT(addIncomingClient(QString)));
        connect(udpServer, SIGNAL(connectionOut(QString)), this, SLOT(removeIncomingClient(QString)));

        // Only Select 1 from the following 2 signal-slot method
        // To get better performance for UI, mask the slot SLOT(updateIncomingData(int,QByteArray)
        // To show Rx data in real-time, mask the slot SLOT(updateIncomingData(int))
        //connect(udpServer, SIGNAL(newDataReady(int,QByteArray)), this, SLOT(updateIncomingData(int,QByteArray)));
        connect(udpServer, SIGNAL(newDataReady(int)), this, SLOT(updateIncomingData(int)));

        connect(udpServer, SIGNAL(newDataTx(QHostAddress,int,QByteArray)), this, SLOT(updateTxDataToLog(QHostAddress,int,QByteArray)));
        connect(udpServer, SIGNAL(message(QString)), this, SLOT(updateMessageToLog(QString)));

        // Enable Listen
        on_pushButton_listen_clicked();
    }
}

void UdpServerWidget::unbind()
{
    if(NULL != udpServer)
    {
        disconnect(udpServer, 0 , this , 0);
    }

    udpServer = NULL;
}

void UdpServerWidget::initWidgetFont()
{
}

void UdpServerWidget::initWidgetStyle()
{
    ui->pushButton_listen->setText(tr("Start Listen"));

    // Update Status Color
    ui->label_status->setStyleSheet(BG_COLOR_RED);
    ui->label_status->setText("");

    // Get host IP address
    ui->lineEdit_IP->setText(QNetworkInterface().allAddresses().at(1).toString());
    qDebug() << "All IP Address: " << QNetworkInterface().allAddresses();

    ui->lineEdit_listenPort->setText(QString::number(8080));

    ui->checkBox_hex->setChecked(hexFormatFlag);
    ui->checkBox_autoClear->setChecked(autoClearRxFlag);
}

void UdpServerWidget::on_pushButton_listen_clicked()
{
    if(NULL == udpServer)
    {
        return;
    }

    // Toggle flag
    isServerRunning = !isServerRunning;

    if(true == isServerRunning)
    {
        ui->pushButton_listen->setText(tr("Stop Listen"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_GREEN);

        udpServer->initSocket(QHostAddress::Any, ui->lineEdit_listenPort->text().toInt());
    }
    else
    {   ui->pushButton_listen->setText(tr("Start Listen"));

        // Update Status Color
        ui->label_status->setStyleSheet(BG_COLOR_RED);

        udpServer->closeSocket();
    }
}

void UdpServerWidget::on_pushButton_send_clicked()
{
    uint32_t txLen = 0;
    uint8_t tempBuf[10000] = {0};
    QByteArray tempTxBuf;
    QUtilityBox utilityBox;

    if(NULL == udpServer)
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
        for(uint32_t i = 0; i < udpServer->getConnectionCount(); i++)
        {
            sendData(i, tempTxBuf);
        }
    }
    else
    {
        sendData(ui->comboBox_clients->currentIndex(), tempTxBuf);
    }

}

void UdpServerWidget::on_pushButton_clear_clicked()
{
    ui->textEdit_log->clear();
}

void UdpServerWidget::sendData(uint32_t clientIndex, QByteArray &data)
{
    QString logStr;

    //qDebug() << "UdpServerWidget::sendData() clientIndex = " << clientIndex;

    if(NULL == udpServer)
    {
        return;
    }

    if(clientIndex >= udpServer->getConnectionCount())
    {
        logStr = tr("clientIndex is out of connections");
        // Update log
        updateLogData(logStr);
        qDebug() << logStr;

        return;
    }

    udpServer->sendData(clientIndex, data);
}

void UdpServerWidget::sendData(uint32_t clientIndex, const char *data, uint32_t len)
{
    QByteArray txData = QByteArray(data, len);
    sendData(clientIndex, txData);
}

void UdpServerWidget::updateLogData(QString logStr)
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

void UdpServerWidget::addIncomingClient(QString str)
{
    QString logStr;

    logStr.append(tr("New client connected:"));
    logStr.append(str);

    // Update log
    updateLogData(logStr);

    ui->comboBox_clients->insertItem(ui->comboBox_clients->count() - 1, str);
}

void UdpServerWidget::removeIncomingClient(QString str)
{
    QString logStr;

    logStr.append(tr("Client removed:"));
    logStr.append(str);

    // Update log
    updateLogData(logStr);

    ui->comboBox_clients->removeItem(ui->comboBox_clients->findText(str));
}

void UdpServerWidget::updateIncomingData(int clientIndex)
{
    QString logStr;

    rxDataBuf.clear();
    if(udpServer->getUndealData(rxDataBuf))
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
    else
    {
        qDebug() << "UdpServerWidget::udpServer->getUndealData(rxDataBuf) no data";
    }
}

void UdpServerWidget::updateIncomingData(int clientIndex, QByteArray data)
{
    QString logStr;

    if(!data.isEmpty())
    {
        logStr.append(tr("Rx data from %1").arg(ui->comboBox_clients->itemText(clientIndex)));
        // Update log
        updateLogData(logStr);

        logStr.clear();
        logStr.append(tr("Rx Data:"));
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

        // Emit signal, new comming data received
        emit newDataReady(clientIndex, data);
    }
}

void UdpServerWidget::updateTxDataToLog(QHostAddress address, int port, QByteArray data)
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

void UdpServerWidget::updateMessageToLog(const QString &msg)
{
    QString logStr;

    logStr.append(msg);

    // Update log
    updateLogData(logStr);
}

void UdpServerWidget::on_pushButton_reset_clicked()
{
    if(NULL == udpServer)
    {
        return;
    }

    udpServer->resetTxRxCnt();
}

void UdpServerWidget::updateUI()
{
    static uint32_t lastRxPacketCnt = 0;
    static uint32_t lastTxPacketCnt = 0;

    if(NULL == udpServer)
    {
        return;
    }

    ui->label_rxPacketCnt->setText(QString::number(udpServer->getRxDiagramCnt()));
    ui->label_txPacketCnt->setText(QString::number(udpServer->getTxDiagramCnt()));
    ui->label_rxPacketCntAvg->setText(QString::number(udpServer->getRxDiagramCnt() - lastRxPacketCnt));
    ui->label_txPacketCntAvg->setText(QString::number(udpServer->getTxDiagramCnt() - lastTxPacketCnt));
    ui->label_rxBytesCnt->setText(QString::number(udpServer->getTotalRxBytes()));
    ui->label_txBytesCnt->setText(QString::number(udpServer->getTotalTxBytes()));

    lastRxPacketCnt = udpServer->getRxDiagramCnt();
    lastTxPacketCnt = udpServer->getTxDiagramCnt();
}

void UdpServerWidget::on_checkBox_hex_clicked(bool checked)
{
    hexFormatFlag = checked;
}

void UdpServerWidget::on_checkBox_autoClear_clicked(bool checked)
{
    autoClearRxFlag = checked;
}
