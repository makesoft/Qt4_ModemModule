#ifndef MODEM_MODULE_H
#define MODEM_MODULE_H

// modem_module.h
// Module for sending SMS from local GSM/3G modem
//
// [SMS SENDING]
// For send SMS you need call function [sendSMSList] with pointers to sms objects
// or standalone sms function [sendSMS] with pointer to sms and bool value [isCyrillic]
//
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


#include <QObject>
#include <QStringList>
#include <QThread>
#include <QTimer>

#include "QtExtSerialPort/qextserialenumerator.h"
#include "QtExtSerialPort/qextserialport.h"

enum TYPE_SMS_STATE{
    SMS_UNSENDED    = 0,
    SMS_SENDED      = 1,
    SMS_ERROR       = 2,
    SMS_TIMEOUT     = 3
};

enum TYPE_MODEM_MODE {
    MODEM_MODE_CIPHER   = 0,
    MODEM_MODE_TEXT     = 1
};

class SMS
 {
public:
    explicit SMS(QString message="", QString cellphone="");
    SMS& operator=(const SMS &right);
    QString getMessage() {return message;}
    QString getCellphone() {return cellphone;}
    TYPE_SMS_STATE getCurrentState () {return currentState;}

    void setCurrentState (TYPE_SMS_STATE typeSMS);
    bool isSended;
protected:
    QString cellphone;
    QString message;
    TYPE_SMS_STATE currentState;
};


class ModemModule : public QThread
{
    Q_OBJECT
public:
    explicit ModemModule(QString modemDevice, QObject *parent = 0);
    ~ModemModule();
    void sendSMS (SMS *newSms, bool isCyrillic=false);
    void sendSMSList (QList<SMS*> lstSms, bool isCyrillic=false, bool isAutoSend=true);
    void nextSMSSend();
    void setModemMode (TYPE_MODEM_MODE typeMode);
signals:
    void error (QString error);
    void operationSendSuccesfull ();
    void operationSingleSMSSuccesfull(SMS *sendedSMS);
protected slots:
    void readyRead();
    void nextStepOperation ();
    void timeoutWaiting ();
    void checkCurrentSmsInlist ();

private:
    void run();
    void endCommandToPort();
    int stepOperation;
    bool isCyrillic, modemModeSetted, isRepeatSend;
    QTimer *timer, *timerSMS;

    QextSerialPort *port;
    SMS *currentSMS;
    QList<SMS*> lstSmsNeedSend;

    QString curModemDevice, lastRespones;
    QStringList needOperations;

    QString getPDU(SMS *curSms);
    QString textToHex (QString inStr);
    QString cellphoneToHex (QString phone);

    const int timerWaitingCount;

    TYPE_MODEM_MODE currModeType;

    bool isAutoSend;
};

#endif // MODEM_MODULE_H
