#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("NadiSoft");
    QCoreApplication::setOrganizationDomain("NadiSoft.com");
    QCoreApplication::setApplicationName("ZiLOG's USBSmartCable Server");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
