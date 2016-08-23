#ifndef CPMDRIVE_H
#define CPMDRIVE_H

#include "cpmrdsk.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QTextStream>

QTextStream& operator << (QTextStream& ots, const dir_t &dir);

class CPMDrive : public QObject
{
    Q_OBJECT
public:
    explicit CPMDrive(const QString name, quint16 secsz, const dpb_t &dpb, const quint8 *xlt, iomode_t mode, QObject *parent);
    ~CPMDrive();

    const QString&  name()   const { return _name;}
    quint16         secsz()  const { return _secsz;}
    quint16         bshift() const { return _dpb.bsh;}
    quint16         blkmsk() const { return _dpb.blm;}
    quint16         blksz()  const { return secsz() << bshift();}
    quint16         maxsys() const { return _dpb.off;}
    quint16         maxdir() const { return _dpb.drm + 1;}
    quint16         maxsec() const { return _dpb.spt;}
    quint16         maxblk() const { return _dpb.dsm + 1;}
    quint16         extcnt() const { return maxblk() < 256 ? 16:8;}
    quint32         maxext() const { return extcnt() * blksz();}
    quint16         maxtrk() const { return (maxblk() * blksz() / secsz() + maxsec() -1) / maxsec() + maxsys();}
    quint16         datablk()const { return (maxdir() * sizeof(dir_t) + blksz() -1 ) / blksz();}
    quint16         dataidx()const { return datablk() / secsz() + maxsys() * maxsec();}
    quint16         secidx(quint16 track, quint16 sect) const { return ((track - maxsys()) * maxsec() + sect) & blkmsk();}
    quint16         xlt(quint16 s) const;
    quint16         getBlockNo( quint16 track, quint16 sec) const { return ((track - maxsys()) * maxsec() + sec) >> bshift();}
    bool            isDir( quint16 block) const { return block < 16 && (_dpb.al1 | (_dpb.al0 << 8)) & (0x8000 >> block);}

    virtual bool open() = 0;
    virtual bool close()= 0;
    virtual bool read(quint16 track, quint16 sectr, char* data)= 0;
    virtual bool write(quint16 track, quint16 sectr, const char* data)= 0;

    QTextStream &log() { return _log;}
private:
    const QString       _name;
    const quint16       _secsz;
    const dpb_t         _dpb;
    iomode_t            _mode;
    QVector<quint16>    _xlt;
    QTextStream         _log;
};


#endif // CPMDRIVE_H
