
#include "smartcableserver.h"
#include <QSettings>

SmartCableServer::SmartCableServer(QObject *parent)
    :QTcpServer(parent)
{
    QSettings settings;
    settings.beginGroup("TCP-Server");

    _port = settings.value("Port", (quint16) 4040U).toUInt();
    _addr = settings.value("Address", QHostAddress(QHostAddress::LocalHost).toString()).toString();
    bool accept = settings.value("Accept", false).toBool();
    if(!listen(_addr, _port) && accept)
    {
        QMessageBox::critical(this, tr("SmartCable Server")
                              tr("Unable to start the server: %1.")
                              .arg(errorString()))
        pauseAccepting();
    else
        resumeAccepting();

    settings.endGroup();
}

SmartCableServer::~SmartCableServer()
{
    QSettings settings;
    settings.beginGroup("TCP-Server");

    settings.setValue("Listen", isListening() && );
    settings.setValue("Port", _port);
    settings.setValue("Address",_addr.toString());

    settings.endGroup();

}
