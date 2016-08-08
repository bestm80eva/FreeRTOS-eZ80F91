#ifndef CPMDRIVE_H
#define CPMDRIVE_H

#include "cpmrdsk.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QMutex>
#include <QFile>

class CPMDir : public dir_t
{
public:
    CPMDir();
    ~CPMDir();
    operator dir_t&() { return (dir_t&) *this;}
};

class CPMDrive : public QObject
{
    Q_OBJECT
public:
    CPMDrive(const QString &path, uint16_t sectsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode);
    virtual ~CPMDrive();

    bool isOpen() const { return _file.isOpen();}
    qint16 tracks() const { return _maxtrack;}
    qint16 secsz() const { return _sectsz; }

    bool open();
    bool close();
    bool read(qint16 track, qint16 sectr, char* data);
    bool write(qint16 track, qint16 sectr, const char* data);
    const QString& name () const {return _path;}

protected:
    quint16 xlt(quint16 s) const;

private:
    iomode_t              _mode;
    const QString       _path;
    const dpb_t         _dpb;
    QVector<quint16>    _xlt;
    QMutex              _mutex;
    QFile               _file;
    qint16              _sectsz;
    qint16              _maxtrack;
    QVector<CPMDir>     _dir;
};

#endif // CPMDRIVE_H
