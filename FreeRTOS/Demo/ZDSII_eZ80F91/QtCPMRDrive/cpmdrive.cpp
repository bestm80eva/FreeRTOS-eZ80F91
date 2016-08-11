
#include "cpmdrive.h"

#include <QMutexLocker>
#include <QDir>

CPMDir::CPMDir()
{
    memset((dir_t*)this,0,sizeof(dir_t));
    status = UNUSED;
}

CPMDir::~CPMDir()
{

}

QTextStream& operator << (QTextStream& ots, const CPMDir &dir)
{
    char name[13];
    strncpy(name,(const char*)dir.name,8);
    name[8] = '.';
    strncpy(name+9,(const char*)dir.next,3);
    name[12] = 0;

    ots << "St=" << qSetFieldWidth(2) << hex << (unsigned short) dir.status << dec << qSetFieldWidth(0);
    ots << ", Fn=" << qSetFieldWidth(13) << name << qSetFieldWidth(0);
    ots << ", Xh/l=" << dir.Xh << "/" << dir.Xl;
    ots << ", Rc/Bc=" << dir.Rc << "/" << dir.Bc;
    ots << qSetFieldWidth(dir.DType() == 8 ? 6:4);
    for(int i = 0; i < dir.DType(); i++)
        ots << dir.getblk(i);
    ots << qSetFieldWidth(0);
    return ots;
}

CPMDir8::CPMDir8()
    :CPMDir()
{
}

CPMDir8::~CPMDir8()
{
}

CPMDir16::CPMDir16()
    :CPMDir()
{
}

CPMDir16::~CPMDir16()
{
}


CPMDrive::CPMDrive(const QString &path, uint16_t sectsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode)
    :QObject(0)
    ,_mode(mode)
    ,_path(path)
    ,_dpb(dpb)
    ,_xlt(secptrk())
    ,_mutex(QMutex::Recursive)
    ,_open(false)
    ,_sectsz(sectsz)
    ,_cache(blkmax()+1)
    ,_log(stdout)
{

    int n;
    if(xlt)
        for(n = 0; n < secptrk(); n++)
        {
            Q_ASSERT(xlt[n] && xlt[n] <= secptrk());
            _xlt [xlt[n]-1] = n;
        }
    else
        for(n = 0; n < secptrk(); n++)
            _xlt[n] = n;
}

CPMDrive::~CPMDrive()
{

}

bool CPMDrive::open()
{
    QDir  dir(_path);
    dir.setFilter(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot|QDir::Readable|QDir::Writable);
    QFileInfoList list = dir.entryInfoList();

    bool result = dir.exists();
    if(result && !_open)
    {
        QMutexLocker locker(&_mutex);
        QDir  dir(_path);
        dir.setFilter(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot|QDir::Readable|QDir::Writable);
        QFileInfoList list = dir.entryInfoList();

        uint16_t block;
        uint16_t diridx=0;
        _open = true;

        char tmp[blksz()];
        memset(tmp,UNUSED,sizeof(tmp));
        for(block = 0; block < datablk(); block++)
            writeabs(block,0,tmp,blksz());

        CPMDir *d;
        if(blktype() == 16)
            d = new CPMDir16;
        else
            d = new CPMDir8;
        Q_ASSERT(d);

        qint32 maxexsz = blktype() * blksz();   // max byte/extend


        for (int i = 0; i < list.size() &&  // for each file on dir
             block < blkmax() &&            // at least one block free
             diridx < dirmax(); ++i)        // at least one extend free
        {
            // cpmname[.cpmext][.~userno]

            QFileInfo fi = list.at(i);      // file info
            QString fulname = fi.fileName();
            QStringList part = fulname.split('.');

            QString cpmname = part.at(0);   // 1st part cpm-base name
            QString cpmext = "";            // optional cpm extentions
            QString userno = "0";           // optional user/status

            if(cpmname.size() > 8)          // skip if to long
                continue;

            cpmname = cpmname.leftJustified(8);

            part.removeFirst();

            if(!part.isEmpty() && part.first()[0] != '~')
            {
                cpmext = part.first();
                part.removeFirst();
                if(cpmext.size() > 3)
                    continue;
            }

            cpmext = cpmext.leftJustified(3);

            if(!part.isEmpty() && part.first()[0] == '~')
            {
                userno = part.first().mid(1);
                part.removeFirst();
            }

            if(!part.isEmpty())
                continue;

            bool ok;
            uint8_t cpmuser = userno.toUShort(&ok);
            if(!ok)
                continue;

            uint16_t extend = 0;                    // extent for this file
            qint64 offset = 0;                      // offset processed

            QFile  file(fi.absoluteFilePath());
            if(!file.open(QIODevice::ReadOnly))
                continue;


            // fill up all needed extends
            do {
                memset(&(dir_t&)*d, 0, sizeof(dir_t));
                // extend setup
                memcpy(&d->name,cpmname.toLocal8Bit().data(),8);
                memcpy(&d->next,cpmext.toLocal8Bit().data(),3);
                d->status = cpmuser;
                d->Xl = (extend & 0x1F);
                d->Xh = (extend >> 5) & 0x3F;

                qint64 extbytes = fi.size() - offset;   // bytes left ...
                if(extbytes > maxexsz)
                    extbytes = maxexsz;                 // on this extend

                // sectors on last block
                if(extbytes)
                {
                    qint16 x = (extbytes+127)  / secsz();// ((extbytes % blksz()) + secsz() - 1) / secsz();

                    d->Rc = x;// ? x : 0x80;
                    d->Bc = 0; // CP/M 2.2

                    // Assign blocks to extend
                    for(int i = 0; i < d->DType() && extbytes > 0; i++)
                    {
                        if(!_cache[block])
                            _cache[block] = new char[blksz()];
                        Q_ASSERT(_cache[block]);

                        memset(_cache[block],0x1A,blksz());
                        file.read(_cache[block],blksz());
                        d->setblk(i, block++);
                        extbytes -= blksz();
                        offset += blksz();
                    }
                }
                _log << "add:" << *d << endl;

                quint16 block = diridx * sizeof(dir_t) / blksz();
                quint16 off   = (diridx * sizeof(dir_t)) % blksz();
                Q_ASSERT(block < datablk());
                Q_ASSERT(off <= (blksz() - sizeof(dir_t)));

                writeabs( block, off, (const char*)&(dir_t&)*d, sizeof(dir_t));

                diridx++;
                extend++;

            } while(offset < fi.size() && diridx <= _dpb.drm );
            file.close();
        }
        result = true;
        delete d;
    }
    return result;
}

bool CPMDrive::close()
{
    QMutexLocker locker(&_mutex);
    _open = false;
    for(int n = 0; n < _cache.count(); n++)
        if(_cache.at(n))
        {
            delete [] _cache.at(n);
            _cache[n] = 0;
        }
    return true;
}

bool CPMDrive::readsys(quint16 track, quint16 sectr, char *data)
{
    Q_ASSERT(track < _dpb.off && sectr <= _dpb.spt);
    memset(data,0,secsz());
    quint16 offset = (track * _dpb.spt + sectr -1) * secsz();
    QFile sys(_path+"/.cpm.sys");
    bool res = sys.open(QIODevice::ReadOnly);

    res = sys.seek(offset) && sys.read(data,secsz()) != -1;
    sys.close();
    return res;
}

bool CPMDrive::writesys(quint16 track, quint16 sectr, const char *data)
{
    Q_ASSERT(track < _dpb.off && sectr <= _dpb.spt);

    quint16 offset = (track * _dpb.spt + sectr -1) * secsz();
    QFile sys(_path+"/.cpm.sys");
    bool res = sys.open(QIODevice::WriteOnly|QIODevice::WriteOnly);

    res = sys.seek(offset) && sys.write(data,secsz()) == secsz();
    sys.close();
    return res;
}

bool CPMDrive::readabs(quint16 block, quint16 offs, char *data, quint16 sz)
{
    QMutexLocker locker(&_mutex);
    bool result = isOpen();
    Q_ASSERT( block <= blkmax());
    Q_ASSERT((offs + sz) <= blksz());

    if(result)
    {
        _log << "RABS: b/o/s " <<  block << '/' << offs << '/' << sz << endl;

        if(!_cache[block])
            _cache[block] = new char[blksz()]();

        Q_ASSERT(_cache[block]);

        memcpy(data, _cache[block] + offs, sz);
    }
    return result;
}

bool CPMDrive::writeabs(quint16 block, quint16 offs, const char *data, quint16 sz)
{
    QMutexLocker locker(&_mutex);
    bool result = isOpen();
    Q_ASSERT( block < blkmax());
    Q_ASSERT((offs + sz) <= blksz());

    if(result)
    {
        _log << "WABS: b/o/s " <<  block << '/' << offs << '/' << sz << endl;

        if(!_cache[block])
            _cache[block] = new char[blksz()]();


        Q_ASSERT(_cache[block]);

        memcpy(_cache[block] + offs, data, sz);
    }
    return result;
}

bool CPMDrive::read(quint16 track, quint16 sectr, char *data)
{
    bool result = false;
    Q_ASSERT( track < tracks());
    Q_ASSERT( sectr > 0 &&  sectr <= _dpb.spt);

    _log << "RCPM: t/s " <<  track << '/' << sectr << endl;

    if(track < _dpb.off)
        result = readsys( track, sectr, data);
    else
        result = readabs(getblk(track, xlt(sectr)), getoff(track, xlt(sectr)), data, secsz());
    return result;
}

bool CPMDrive::write(quint16 track, quint16 sectr, const char *data)
{
    bool result = false;
    Q_ASSERT( track < tracks());
    Q_ASSERT( sectr > 0 &&  sectr <= _dpb.spt);

    _log << "WCPM: t/s " <<  track << '/' << sectr << endl;

    if(track < _dpb.off)
        result = writesys( track, sectr, data);
    else
        result = writeabs(getblk(track, xlt(sectr)), getoff(track, xlt(sectr)), data, secsz());
    return result;
}


quint16 CPMDrive::xlt(quint16 s) const
{
    Q_ASSERT(s > 0 && s <= _dpb.spt);

    return _mode & LINEAR ? (s-1) : _xlt[s-1];
}
