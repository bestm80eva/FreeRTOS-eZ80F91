#ifndef CPMDRIVE_H
#define CPMDRIVE_H

#include "cpmrdsk.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QMutex>
#include <QFile>
#include <QTextStream>

class CPMDir : public dir_t
{
public:
    CPMDir();
    virtual ~CPMDir();
    virtual quint16 DType() const = 0;
    virtual void setblk(quint16 idx, quint16 blk) = 0;
    virtual quint16 getblk(quint16 idx) const = 0;
    operator dir_t&() { return (dir_t&) *this;}
    friend QTextStream& operator << (QTextStream& ots, const CPMDir &dir);
};

class CPMDir8 : public CPMDir
{
public:
    CPMDir8();
    ~CPMDir8();
    virtual quint16 DType() const { return 8; }
    virtual void setblk(quint16 idx, quint16 blk) { Q_ASSERT( idx < DType()); Al.w[idx] = blk;}
    virtual quint16 getblk(quint16 idx) const { Q_ASSERT( idx < DType()); return Al.w[idx];}
};

class CPMDir16 : public CPMDir
{
public:
    CPMDir16();
    ~CPMDir16();
    virtual quint16 DType() const { return 16; }
    virtual void setblk(quint16 idx, quint16 blk) { Q_ASSERT( idx < DType()); Al.b[idx] = blk;}
    virtual quint16 getblk(quint16 idx) const { Q_ASSERT( idx < DType()); return Al.b[idx];}
};

class CPMDrive : public QObject
{
    Q_OBJECT
public:
    CPMDrive(const QString &path, uint16_t sectsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode);
    virtual ~CPMDrive();

    bool    isOpen() const { return _open;}
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

    quint16 getblk( qint16 track, qint16 sectr) const { return ( (track - sysmax()) * secptrk() + sectr) >> _dpb.bsh; }
    quint16 getoff( qint16 track, qint16 sectr) const { return (((track - sysmax()) * secptrk() + sectr) &  _dpb.blm) * secsz() ;}
private:
    iomode_t            _mode;
    const QString       _path;
    const dpb_t         _dpb;
    QVector<quint16>    _xlt;
    QMutex              _mutex;
    bool                _open;
    qint16              _sectsz;
    QVector<char*>      _cache;
    QTextStream         _log;
};

#endif // CPMDRIVE_H
