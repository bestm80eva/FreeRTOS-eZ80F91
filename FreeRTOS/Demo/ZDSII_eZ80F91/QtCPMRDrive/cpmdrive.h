#ifndef CPMDRIVE_H
#define CPMDRIVE_H

#include "cpmrdsk.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QFileSystemWatcher>

class CPMDrive : public QObject
{
    Q_OBJECT
public:
    CPMDrive(const QString &path, uint16_t sectsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode, QObject *parent=0);
    virtual ~CPMDrive();

    bool    isOpen() const { return _cache != 0;}
    quint16 secptrk()const { return _dpb.spt;}
    quint16 dirmax() const { return _dpb.drm + 1;}
    quint16 blkmax() const { return _dpb.dsm + 1;}
    quint16 sysmax() const { return _dpb.off;}
    quint64 dtamax() const { return blkmax() * blksz();}
    quint16 tracks() const { return (dtamax() / secsz() + secptrk() -1) / secptrk() + sysmax();}
    quint16 secsz()  const { return _sectsz; }
    quint16 blksz()  const { return secsz() << _dpb.bsh;}
    quint16 datablk()const { return ((dirmax() * sizeof(dir_t)) + blksz() - 1) / blksz();}
    quint16 blktype()const { return _dpb.dsm > 255 ? 8:16;} // block no size

    bool open();
    bool close();
    bool read(quint16 track, quint16 sectr, char* data);
    bool write(quint16 track, quint16 sectr, const char* data);
    const QString& name () const {return _path;}

protected:
    quint16 xlt(quint16 s) const;
    bool readsys(quint16 track, quint16 sectr, char* data);
    bool writesys(quint16 track, quint16 sectr, const char *data);
    bool readabs(quint16 block, quint16 offs, char* data, quint16 sz);
    bool writeabs(quint16 block, quint16 offs, const char* data, quint16 sz);
    bool writedir(quint16 block, quint16 offs, const char* data, quint16 sz);

    quint16 getblk( qint16 track, qint16 sectr) const { return ( (track - sysmax()) * secptrk() + sectr) >> _dpb.bsh; }
    quint16 getoff( qint16 track, qint16 sectr) const { return (((track - sysmax()) * secptrk() + sectr) &  _dpb.blm) * secsz() ;}

    int addHostname(const QByteArray &cpmname, const QString& hostname);
    int addHostname(const QByteArray &cpmname);
    bool delHostname(const QByteArray &cpmname);
    bool renHostname(const QByteArray &cpmfrom, const QByteArray &cpmto);

    QString getHostname(const QByteArray &cpmname);

    qint32 findBlock(QString& hostfile, quint16 block);

private slots:
    void directoryChanged(const QString &path);

private:
    void watch( bool x)
    {
        if(x)
            connect(&_watch,SIGNAL(directoryChanged(QString)),this,SLOT(directoryChanged(QString)));
        else
            disconnect(&_watch,SIGNAL(directoryChanged(QString)),this,SLOT(directoryChanged(QString)));
    }

    iomode_t            _mode;
    const QString       _path;
    const dpb_t         _dpb;
    QVector<quint16>    _xlt;
    QMutex              _mutex;
    qint16              _sectsz;
    QMap<quint16,char*> *_cache;
    QMap<QByteArray, QString> *_hostname;
    QTextStream         _log;
    QFileSystemWatcher  _watch;
    bool                _changed;
};

#endif // CPMDRIVE_H
