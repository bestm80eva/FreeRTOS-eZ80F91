
#include "cpmdrive.h"

#include <QMutexLocker>
CPMDir::CPMDir()
{
    memset((dir_t*)this,UNUSED,sizeof(dir_t));
}

CPMDir::~CPMDir()
{

}

CPMDrive::CPMDrive(const QString &path, uint16_t sectsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode)
    :QObject(0)
    ,_mode(mode)
    ,_path(path)
    ,_dpb(dpb)
    ,_xlt(_dpb.spt)
    ,_file("../CPM/CPMRDisk/disks/" + path)
    ,_sectsz(sectsz)
    ,_maxtrack((_sectsz << _dpb.bsh) * (_dpb.dsm + 1) / _sectsz / _dpb.spt + _dpb.off)
    ,_dir(_dpb.drm)
{
    int n;
    if(xlt)
        for(n = 0; n < _dpb.spt; n++)
        {
            Q_ASSERT(xlt[n] && xlt[n] <= _dpb.spt);
            _xlt [xlt[n]-1] = n;
        }
    else
        for(n = 0; n < _dpb.spt; n++)
            _xlt[n] = n;
}

CPMDrive::~CPMDrive()
{

}

bool CPMDrive::open()
{
    bool result = isOpen();
    if(!result)
    {
        QMutexLocker locker(&_mutex);
        result = isOpen();
        if(!result)
           result = _file.open( _mode & READONLY ? QIODevice::ReadOnly:QIODevice::ReadWrite);
    }
    return result;
}

bool CPMDrive::close()
{
    bool result = isOpen();
    if(result)
    {
        QMutexLocker locker(&_mutex);
        result = isOpen();
        if(result)
           _file.close();
    }
    return result;
}

bool CPMDrive::read(qint16 track, qint16 sectr, char *data)
{
    bool result = isOpen();
    Q_ASSERT( track < tracks());
    Q_ASSERT( sectr > 0 &&  sectr <= _dpb.spt);

    if(result)
    {
        qint64 offset = (track * _dpb.spt + xlt(sectr)) * _sectsz;

        if(_file.size() < (offset+_sectsz))
            _file.resize(offset+_sectsz);

        result = _file.seek(offset) && _sectsz == _file.read(data, _sectsz);
    }
    return result;
}

bool CPMDrive::write(qint16 track, qint16 sectr, const char *data)
{
    bool result = isOpen();
    Q_ASSERT( track <= tracks());
    Q_ASSERT( sectr > 0 &&  sectr <= _dpb.spt);

    if(result)
    {
        qint64 offset = (track * _dpb.spt + xlt(sectr)) * _sectsz;

        if(_file.size() < (offset+_sectsz))
            _file.resize(offset+_sectsz);

        result = _file.seek(offset) && _sectsz == _file.write(data, _sectsz);
    }
    return result;
}

quint16 CPMDrive::xlt(quint16 s) const
{
    Q_ASSERT(s && s <= _dpb.spt);

    return _mode & LINEAR ? (s-1) : _xlt[s-1];
}
