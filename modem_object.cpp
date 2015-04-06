#include "modem_object.h"

#include <QDebug>
#include <iostream>
#include <QString>
#include <QBuffer>
using namespace std;

ModemObject::ModemObject(QObject *parent)
    : QObject(parent)
{
    SMS *newSms = new SMS (trUtf8("Все, последний раз :)"), "+1234567890");
    SMS *newSms1 = new SMS (trUtf8("Все, последний раз :)"), "+1234567890");
    SMS *newSms2 = new SMS (trUtf8("Все, последний раз :)"), "+1234567890");

    listSMS.push_back(newSms);
    listSMS.push_back(newSms1);
    listSMS.push_back(newSms2);

    modem = new ModemModule ("/dev/ttyUSB0"); // your mobile modem
    modem->sendSMSList(listSMS, true);

    connect (modem,SIGNAL(error(QString)), this, SLOT(modemError (QString)));
    connect (modem,SIGNAL(operationSendSuccesfull()), this, SLOT(modemOperationSendSuccesfull()));

    modem->start();
}

void ModemObject::modemOperationSendSuccesfull()
{
    for (int i=0; i<listSMS.size();i++)
        qDebug() << QString("SMS to: %1\n with message: %2\n is %3!").
                    arg(listSMS.at(i)->getCellphone()).
                    arg(listSMS.at(i)->getMessage()).
                    arg(listSMS.at(i)->isSended ? QString("sended!")
                                                : QString("not sended!"));
}

void ModemObject::modemError(QString err)
{
    qDebug() << err;
}

ModemObject::~ModemObject()
{
    for (int i=0; i<listSMS.size();i++)
        delete listSMS[i];
}





























