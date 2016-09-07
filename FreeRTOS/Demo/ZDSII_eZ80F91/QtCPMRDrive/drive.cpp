#include "drive.h"

#include <QTextStream>

CPMDrive::CPMDrive( const QString name, uint16_t secsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode, QObject *parent = 0)
    :QObject(parent)
    ,_name(name)
    ,_secsz(secsz)
    ,_dpb(dpb)
    ,_mode(mode)
    ,_xlt(dpb.spt)
    ,_log(stdout)
{
    unsigned n;

    if(xlt)
        for(n = 0; n < maxsec(); n++)
        {
            Q_ASSERT(xlt[n] && xlt[n] <= maxsec());
            _xlt [xlt[n]-1] = n;
        }
    else
        for(n = 0; n < maxsec(); n++)
            _xlt[n] = n;
}

CPMDrive::~CPMDrive()
{

}

quint16 CPMDrive::xlt(quint16 s) const
{
    Q_ASSERT(s > 0 && s <= _dpb.spt);

    return _mode & LINEAR ? (s-1) : _xlt[s-1];
}

QTextStream& operator << (QTextStream& ots, const dir_t &dir)
{
    char name[13];
    strncpy(name,(const char*)dir.name,8);
    name[8] = '.';
    strncpy(name+9,(const char*)dir.next,3);
    name[12] = 0;

    ots << "St=" << qSetFieldWidth(2) << hex << (unsigned short) dir.status << dec << qSetFieldWidth(0);
    ots << "h, Fn=" << qSetFieldWidth(13) << name << qSetFieldWidth(0);
    ots << ", Xhl=" << qSetFieldWidth(5) << dec << ((dir.Xh << 5) | dir.Xl);
    ots << ", Rc/Bc=" << qSetFieldWidth(3) << dec << dir.Rc << qSetFieldWidth(0) << "/" << qSetFieldWidth(3) << dec << dir.Bc;
    ots << ", blks=" << qSetFieldWidth(4);

    for(int i = 0; i < 16; i++)
    {
        ots << (quint16) dir.Al.b[i];
    }
    ots << qSetFieldWidth(0) << " / ";
    ots << qSetFieldWidth(6);
    for(int i = 0; i < 8; i++)
    {
        ots << dir.Al.w[i];
    }

    ots << qSetFieldWidth(0);
    return ots;
}

bool CPMDrive::isDir(quint32 abssec) const
{
    quint16 al = _dpb.al1 | (_dpb.al0 << 8);
    quint16 block = getBlock(abssec);
    quint16 msk = block < 16 ? (0x8000 >> block):0;
    return (msk & al) != 0;
}
