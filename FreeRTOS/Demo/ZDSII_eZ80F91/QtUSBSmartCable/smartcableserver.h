#ifndef SMARTCABLESERVER_H
#define SMARTCABLESERVER_H

#include <QTcpServer>
#include <qglobal.h>

class SmartCableServer : public QTcpServer
{
    Q_OBJECT
public:
    SmartCableServer(QObject *parent = Q_NULLPTR);
    virtual ~SmartCableServer();

protected:

private:
     quint16 _port;
     QHostAddress _addr;
};

#endif // SMARTCABLESERVER_H
