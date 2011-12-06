#include <QtGui/QApplication>
#include "pop3.h"
#include "Client.h"
#include <QtGlobal>
#include <QtDebug>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("CP-1251"));
    QApplication a(argc, argv);
    Pop3 w;
    w.show();
    return a.exec();
}

