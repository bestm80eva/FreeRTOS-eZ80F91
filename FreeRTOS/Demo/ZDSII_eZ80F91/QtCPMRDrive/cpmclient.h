#ifndef CPMCLIENT_H
#define CPMCLIENT_H

#include "cpmdrive.h"

#include <QWidget>
#include <QHostAddress>
#include <QMap>

class CPMClient : public QWidget
{
    Q_OBJECT
public:
    explicit CPMClient(const QString &host, QWidget *parent = 0);
    ~CPMClient() {}

    bool req(const hdr_t& request, QByteArray& resp);
    const QString& host() const { return _host;}

    unsigned requests() const { return _requests; }
    unsigned indata() const { return _reqdata; }
    unsigned outdata() const { return _rspdata; }

signals:
    void adddrive(CPMClient* client, CPMDrive* drive);
    void deldrive(CPMClient* client, CPMDrive* drive);

public slots:
protected:
    unsigned      _requests;
    unsigned long _reqdata;
    unsigned long _rspdata;
private:
    QString                  _host;
    QMap< quint8, CPMDrive*> _drive;
    qint16                   _seqnz;
};

// Q_DECLARE_METATYPE(CPMClient);

#endif // CPMCLIENT_H
