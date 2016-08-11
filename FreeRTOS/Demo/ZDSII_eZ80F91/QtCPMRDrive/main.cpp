#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

#ifdef QT_NO_DEBUG_OUTPUT
#error
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
