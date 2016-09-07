#ifndef HOSTDIRDRIVE_H
#define HOSTDIRDRIVE_H

#include "drive.h"

#include <QMutex>
#include <QMap>
#include <QByteArray>

class HostDirDrive : public CPMDrive
{
    Q_OBJECT
public:
    explicit HostDirDrive(const QString &path, uint16_t secsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode, QObject *parent = 0);
    ~HostDirDrive();

    virtual bool open();
    virtual bool close();
    virtual bool read(quint16 track, quint16 sect, char* data);
    virtual bool write(quint16 track, quint16 sect, const char* data);

    bool isOpen() const { return _open;}
signals:

public slots:
protected:
    bool        _open;
    QString     getHostname(const dir_t& ext);
    dir_t &     dir(quint16 idx);
    bool        readdir(quint32 abssec, dir_t* data);
    bool        readsec(quint32 abssec, char* data);
    bool        readsys(quint16 offset, char *data);
    bool        writedir(quint32 abssec, const dir_t *data);
    bool        writesec(quint32 abssec, const char *data);
    bool        writesys(quint16 offset, const char *data);

    qint64 findBlock(quint16 block, QString& hostname);

private:

    QMutex          _mutex;

    QMap<quint32,char*>         _cache;
    QMap<QByteArray, QString>   _hostname;

};

#endif // HOSTDIRDRIVE_H
