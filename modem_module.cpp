// modem_module.cpp
// Module for sending SMS from local GSM/3G modem
//
// [SMS SENDING]
// For send SMS you need call function [sendSMSList] with pointers to sms objects
// or standalone sms function [sendSMS] with pointer to sms
// functions marked sms param [isSended] if sending complete.
//
// after all sms sends class emit signal [operationSendSuccesfull]
// and error if modem not valid
//
// [SMS OBJECT INITIALIZE]
// For create SMS object call constructor with params:
//      message     - text message
//      cellphone   - number to sending
//

#include "modem_module.h"
#include <iostream>
#include <QString>
#include <QBuffer>
#include <QDebug>

SMS::SMS(QString message, QString cellphone) :
    isSended(false)
{
    this->message = message;
    this->cellphone = cellphone;
    currentState = SMS_UNSENDED;
}

SMS& SMS::operator=(const SMS &right)
{
    this->message = right.message;
    this->cellphone = right.cellphone;
    this->isSended = right.isSended;
    this->currentState = right.currentState;
}

void SMS::setCurrentState(TYPE_SMS_STATE typeSMS)
{
    currentState = typeSMS;
}

QString ModemModule::textToHex (QString inStr)
{
     // convert message to PDU format
    QString out;
    QVector<uint> arrUcs4 = inStr.toUcs4();
    for (int i=0;i<arrUcs4.size();i++)
    {
        QString tmpValue = QString::number(arrUcs4.at(i), 16);
        for (int j=tmpValue.size();j<4;j++) tmpValue.insert(0,"0");
        out += tmpValue;
    }
    return out;
}

QString ModemModule::cellphoneToHex (QString phone)
{
    // convert phone to PDU format
    phone.remove("+");
    QString outPhone;
    if ((phone.length() % 2) == 1)
        phone += "F";
    for (int i=0;i<phone.length();i++)
        if ((i % 2) == 1)
        {
            outPhone += phone.at(i);
            outPhone += phone.at(i-1);
        }
    return outPhone;
}

QString ModemModule::getPDU (SMS *curSms)
{
    QString outPDU;

    QString hexMsg = textToHex(curSms->getMessage());
    QString hexPhone = cellphoneToHex(curSms->getCellphone());

    // SMS Center '00' / SMS-SUBMIT '11' / Default number use '00'
    outPDU += "001100";
    // Lenght of number in HEX
    if (hexPhone.size() < 16)
        outPDU.append('0');
    outPDU += QString::number(hexPhone.size(), 16);
    // International
    outPDU += "91";
    // Number for sending in HEX
    outPDU += hexPhone;
    // ID protocol
    outPDU += "00";
    // '0' - Flash SMS; '8' - unicode
    outPDU += "08";
    // time sending '05' - 30 minutes
    outPDU += "05";
    // Lenght of message
    if (hexMsg.size()/2 < 16)
        outPDU.append('0');
    outPDU += QString::number (hexMsg.size()/2, 16);
    // Message
    outPDU += hexMsg;

    return outPDU;
}

void ModemModule::sendSMSList (QList<SMS *> lstSms, bool isCyrillic, bool isAutoSend)
{
    lstSmsNeedSend.clear();

    this->isCyrillic = isCyrillic;
    this->isAutoSend = isAutoSend;

    for (int i=0;i<lstSms.size();i++)
        lstSmsNeedSend.push_back(lstSms.at(i));

    if(lstSmsNeedSend.size() > 0)
        this->sendSMS(lstSmsNeedSend.at(0), isCyrillic);
}

void ModemModule::setModemMode (TYPE_MODEM_MODE typeMode)
{
    currModeType = typeMode;
    timer->start(timerWaitingCount);
    endCommandToPort ();
    port->write(QString("AT+CMGF=%1\r\n").arg(typeMode).toAscii());
}


void ModemModule::sendSMS(SMS *newSms, bool isCyrillic)
{
    if (!modemModeSetted)
        return;

    endCommandToPort ();
    *&currentSMS = NULL;
    *&currentSMS = newSms;

    needOperations.clear();
    if (isCyrillic)
    {
        // we need a size of PDU code
        needOperations.push_back(QString("AT+CMGS=%1\r\n").arg((getPDU(currentSMS).size()-2)/2));
        // genarate PDU
        needOperations.push_back(QString("%1%2").arg(getPDU(currentSMS)).arg(char(26)));
    }
    else
    {
        needOperations.push_back(QString("AT+CMGS=\"%1\"\r\n").arg(currentSMS->getCellphone()));
        needOperations.push_back(QString("%1\%2").arg(currentSMS->getMessage()).arg(char(26)));
    }

    //timer->start(timerWaitingCount);
    stepOperation = 0;
    nextStepOperation();
}

ModemModule::ModemModule(QString modemDevice, QObject *parent) :
    QThread(parent), stepOperation(0), timerWaitingCount(10000),
    isAutoSend(true), modemModeSetted(false), isRepeatSend(false)
{
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timeoutWaiting()));

    timerSMS = new QTimer;
    timerSMS->setInterval(10000); // waiting time for sending
    timerSMS->setSingleShot(true);
    connect(timerSMS, SIGNAL(timeout()),
            this, SLOT(checkCurrentSmsInlist()));

    curModemDevice = modemDevice;
    port = new QextSerialPort(curModemDevice);
    connect(port, SIGNAL(readyRead()), this, SLOT(readyRead()));

    if (!port->open(QIODevice::ReadWrite))
        emit error(QString("Dont open modem on port %1").arg(curModemDevice));
}

void ModemModule::timeoutWaiting ()
{
    if (!modemModeSetted)
    {
        timer->stop();
        setModemMode(currModeType);
        timer->start(timerWaitingCount);
    }
    else
    {
        checkCurrentSmsInlist();
        emit error("Modem doesn't respond. Please check your modem.");
    }
}

void ModemModule::run ()
{

}

void ModemModule::readyRead()
{
    // stop timer and read respones
    timer->stop();

    QByteArray arr;
    arr = port->read(port->bytesAvailable());
    lastRespones = arr;
    lastRespones.replace("\r\n", "");

    qDebug() << lastRespones;

    if (lastRespones.contains("ERROR"))
    {
        currentSMS->isSended = false;
        currentSMS->setCurrentState(SMS_ERROR);
        emit error(port->errorString());
        checkCurrentSmsInlist();
    }
    else if (lastRespones.contains("CMGS"))
    {
        currentSMS->isSended=true;
        currentSMS->setCurrentState(SMS_SENDED);
        timerSMS->start();
    }
    else if (lastRespones.contains(">"))
            nextStepOperation();
    else if (lastRespones.contains("OK"))
    {
        modemModeSetted = true;
        if (lstSmsNeedSend.size() > 0)
            sendSMS(lstSmsNeedSend.at(0), this->isCyrillic);
    }
}

void ModemModule::nextStepOperation()
{
    // check if next step sending or checking new sms
    if (stepOperation < needOperations.size())
    {
            port->flush();
            QByteArray outBytes (needOperations.at(stepOperation).
                                 toStdString().c_str());
            port->write(outBytes);
            port->waitForBytesWritten(1000);
            qDebug() << "current operation:" << needOperations.at(stepOperation);
            stepOperation++;
            timer->start(timerWaitingCount);
    }
    else
    {
        qDebug() << "operations is completed";
    }
}

void ModemModule::endCommandToPort ()
{
    QString eof(char(26));
    port->write (eof.toAscii());
    port->flush();
}

void ModemModule::checkCurrentSmsInlist()
{
    timerSMS->stop();
    timer->stop();
    needOperations.clear();
    endCommandToPort ();

    if (currentSMS->getCurrentState() == SMS_UNSENDED)
        currentSMS->setCurrentState(SMS_TIMEOUT);
    else if (currentSMS->getCurrentState() == SMS_TIMEOUT)
        currentSMS->setCurrentState(SMS_ERROR);

    // remove current sms from list
        for (int i=0;i<lstSmsNeedSend.size();i++)
            if (lstSmsNeedSend.at(i)->getMessage() == currentSMS->getMessage())
            {
                if (currentSMS->getCurrentState() == SMS_TIMEOUT)
                    lstSmsNeedSend.swap(i, lstSmsNeedSend.size()-1);
                else
                    lstSmsNeedSend.erase(lstSmsNeedSend.begin() + i);
                break;
            }

    if(lstSmsNeedSend.size() == 0)
        emit operationSingleSMSSuccesfull(currentSMS);
    else
        if (isAutoSend)
            nextSMSSend();
        else
            if (currentSMS->getCurrentState() == SMS_ERROR ||
                currentSMS->getCurrentState() == SMS_SENDED)
                    emit operationSingleSMSSuccesfull(currentSMS);
            else
                nextSMSSend();
}

void ModemModule::nextSMSSend()
{
    if (lstSmsNeedSend.size() > 0)
        sendSMS(lstSmsNeedSend.at(0), this->isCyrillic);
    else
       emit operationSendSuccesfull();
}


ModemModule::~ModemModule()
{
    if (timer)
        delete timer;
    if (port)
    {
        port->close();
        delete port;
    }
if (currentSMS)
        delete currentSMS;
}
