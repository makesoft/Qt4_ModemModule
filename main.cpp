#include "modem_object.h"
#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    ModemObject *mModem = new ModemObject;
    return a.exec();
}







