#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QObject>
#include "modem_module.h"

class ModemObject : public QObject
{
    Q_OBJECT
public:
    ModemObject (QObject *parent = 0);
    ~ModemObject();

protected slots:
    void modemError(QString err);
    void modemOperationSendSuccesfull();
protected:
    int step;
    ModemModule *modem;
    QList<SMS*> listSMS;
};

#endif // MAINWIDGET_H
