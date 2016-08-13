
#include "cpmdrive.h"

#include <QMutexLocker>
#include <QDir>


QTextStream& operator << (QTextStream& ots, const dir_t &dir)
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
    QString s;
    QTextStream tmp(&s);
    tmp << qSetFieldWidth(6);
    ots << qSetFieldWidth(4);

    for(int i = 0; i < 16; i += 2)
    {
        if(!(i&1))
            tmp << dir.Al.w[i];
        ots << dir.Al.b[i];
    }
    ots << qSetFieldWidth(0);
    ots << " / " << s;
    return ots;
}

CPMDrive::CPMDrive(const QString &path, uint16_t sectsz, const dpb_t &dpb, const uint8_t *xlt, iomode_t mode, QObject *parent)
    :QObject(parent)
    ,_mode(mode)
    ,_path(path)
    ,_dpb(dpb)
    ,_xlt(secptrk())
    ,_mutex(QMutex::Recursive)
    ,_sectsz(sectsz)
    ,_cache(0)
    ,_hostname(0)
    ,_log(stdout)
    ,_watch(QStringList(_path),this)
    ,_changed(false)
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

void CPMDrive::directoryChanged(const QString &path)
{
    _log << "Disk changed " << path << endl;
    _changed = true;
    if(isOpen())
    {
        ;//  close();
    }
}

QString CPMDrive::getHostname(const QByteArray &cpmname)
{
    QString hostname;

    QMap<QByteArray, QString>::const_iterator it = _hostname->find(cpmname);
    if(it == _hostname->end())
    {
        hostname = QString("%1.%2.~%3").arg(cpmname.mid(1,8).trimmed().constData())
                                       .arg(cpmname.mid(9,3).trimmed().constData())
                                       .arg((quint16) cpmname[0]);
        QFile f(_path + '/' + hostname);
        if(!f.exists() && !cpmname[0])
            hostname = hostname.left(hostname.size()-3);
    }
    else
        hostname = it.value();

    return hostname;
}

int CPMDrive::addHostname(const QByteArray &cpmname, const QString& hostname)
{
    (*_hostname)[cpmname] = hostname;
    return _hostname->count();
}

int CPMDrive::addHostname(const QByteArray &cpmname)
{
    bool r = true;
    QString hostname = getHostname(cpmname);

    QFile f(_path + '/' + hostname);
    if(!f.exists())
    {
        watch(false);
        r = f.open(QIODevice::ReadWrite);
        f.close();
        watch(true);
    }
    return r ? addHostname(cpmname, hostname) : -1;
}

bool CPMDrive::delHostname(const QByteArray &cpmname)
{
    bool result = false;
    QString hostname = getHostname(cpmname);
    QFile f(_path + '/' + hostname);

    if(f.exists())
    {
        watch(false);
        result = f.remove();
        watch(true);
    }
    _hostname->remove(cpmname);

    return result;
}

bool CPMDrive::renHostname(const QByteArray &cpmfrom, const QByteArray &cpmto)
{
    bool result;
    QString from = getHostname(cpmfrom);
    QString to   = getHostname(cpmto);
    QFile f(_path + '/' + from);
    result = f.exists();
    if(result)
    {
        watch(false);
        result = f.rename(_path + '/' + to);
        watch(true);
    }
    return result;
}

bool CPMDrive::open()
{
    QDir  dir(_path);
    bool result = dir.exists();

    if(result && !isOpen())
    {
        QMutexLocker locker(&_mutex);
        dir.setFilter(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot|QDir::Readable|QDir::Writable);
        QFileInfoList list = dir.entryInfoList();

        uint16_t block;
        uint16_t diridx=0;

        _cache = new QMap<quint16,char*>;
        Q_ASSERT(_cache);
        _hostname = new QMap<QByteArray, QString>;
        Q_ASSERT(_hostname);

        char tmp[blksz()];
        memset(tmp,0,sizeof(tmp));

        for(block = 0; block < blksz(); block += sizeof(dir_t))
            tmp[block] = UNUSED;

        for(block = 0; block < datablk(); block++)
            writeabs(block,0,tmp,blksz());


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
            dir_t d;

            // fill up all needed extends
            do {
                memset(&d, 0, sizeof(dir_t));
                // extend setup
                memcpy(&d.name,cpmname.toLocal8Bit().data(),8);
                memcpy(&d.next,cpmext.toLocal8Bit().data(),3);
                d.status = cpmuser;
                addHostname(QByteArray((const char*)&d.status,12));
                d.Xl = (extend & 0x1F);
                d.Xh = (extend >> 5) & 0x3F;

                qint64 extbytes = fi.size() - offset;   // bytes left ...
                if(extbytes > maxexsz)
                    extbytes = maxexsz;                 // on this extend

                // sectors on last block
                if(extbytes)
                {
                    qint16 x = (extbytes+127)  / secsz();// ((extbytes % blksz()) + secsz() - 1) / secsz();

                    d.Rc = x;// ? x : 0x80;
                    d.Bc = 0; // CP/M 2.2

                    // Assign blocks to extend
                    for(int i = 0; i < blktype() && extbytes > 0; i++)
                    {
                        if(blktype() == 8)
                            d.Al.w[i] = block++;
                        else
                            d.Al.b[i] = block++;

                        extbytes -= blksz();
                        offset += blksz();
                    }
                }
                _log << "add:" << d << endl;

                quint16 block = diridx * sizeof(dir_t) / blksz();
                quint16 off   = (diridx * sizeof(dir_t)) % blksz();
                Q_ASSERT(block < datablk());
                Q_ASSERT(off <= (blksz() - sizeof(dir_t)));

                writeabs( block, off, (const char*)&d, sizeof(dir_t));

                diridx++;
                extend++;

            } while(offset < fi.size() && diridx <= _dpb.drm );
            file.close();
        }
        result = true;
        watch(true);
    }
    _changed = result;
    return result;
}

bool CPMDrive::close()
{
    QMutexLocker locker(&_mutex);
    watch(false);

    if(_cache)
    {
        QMap<quint16, char*>::iterator it;
        for(it = _cache->begin(); it != _cache->end(); ++it)
        {
            if(it.value())
                delete [] it.value();
        }
        delete _cache;
        _cache = 0;
    }
    if(_hostname)
    {
        delete _hostname;
        _hostname = 0;
    }

    return _changed = true;
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
    watch(false);
    QFile sys(_path+"/.cpm.sys");
    bool res = sys.open(QIODevice::WriteOnly|QIODevice::WriteOnly);
    res = sys.seek(offset) && sys.write(data,secsz()) == secsz();
    sys.close();
    watch(true);
    return res;
}

qint32 CPMDrive::findBlock(QString& hostfile, quint16 block)
{
    qint32 res = -1;
    dir_t  *ext;
    if (block >= datablk())
    {
        for( int i = 0; i < dirmax() && res == -1; i++)
        {
            quint16 dblock = i * sizeof(dir_t) / blksz();
            QMap<quint16, char*>::iterator it = _cache->find(dblock);

            if(it == _cache->end())
                continue;

            ext = (dir_t*)(it.value() + (i * sizeof(dir_t)) % blksz());
            if(ext->status == UNUSED)
                continue;

            int t = blktype();
            for(int e = 0; e < t; e++)
            {
                if(t == 8 )
                {
                    if(block == ext->Al.w[e])
                    {
                        hostfile = getHostname(QByteArray((const char*)ext,12));
                        res = ((ext->Xl + (ext->Xl << 5)) * t + e) * blksz();
                        break;
                    }
                }
                else
                {
                    if(block == ext->Al.b[e])
                    {
                        hostfile = getHostname(QByteArray((const char*)ext,12));
                        res = ((ext->Xl + (ext->Xl << 5)) * t + e) * blksz();
                        break;
                    }
                }
            }
        }
    }
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

        QMap<quint16, char*>::const_iterator i = _cache->find(block);

        if(i == _cache->end())
        {
            QString hostfile;
            qint32 blk = findBlock(hostfile,block);
            if(blk != -1)
            {
                QFile f(_path + '/' + hostfile);
                Q_ASSERT(f.open(QIODevice::ReadOnly));
                qint64 n;
                result = f.seek(blk+offs) && (n = f.read(data,sz)) != -1;
                f.close();
                if(result && n < sz)
                    memset(data+n,0x1A,sz-n);
            }
            else
                memset(data,0x1A,sz);
        }
        else
            memcpy(data, (*_cache)[block] + offs, sz);
    }
    return result;
}

bool CPMDrive::writedir(quint16 block, quint16 offs, const char *data, quint16 sz)
{
    char dsec[sz];

    bool result = readabs(block, offs, dsec, sz) && writeabs(block, offs, data, sz);

    if(result)
    {
        dir_t *o,*d;

        for(o = (dir_t*) dsec, d = (dir_t*) data; d < (dir_t*)(data+sz); d++, o++)
        {
            if(o->status == UNUSED && d->status == UNUSED)
                continue;

            if( memcmp(o, d, 12))
            {
                // deleted file must not change
                Q_ASSERT(o->status != UNUSED || d->status != UNUSED);

                if(o->status == UNUSED)
                    // new file for this extend
                    addHostname(QByteArray((const char*)&d->status,12));
                else if(d->status == UNUSED)
                    // file on this extebd shall be removed
                    delHostname(QByteArray((const char*)&o->status,12));
                else
                    // filename or user has been changed
                    renHostname(QByteArray((const char*)&o->status,12), QByteArray((const char*)&d->status,12));
            }

            for( int b = 0; result && b < blktype(); b++)
            {
                quint16 bno = blktype() == 8 ? d->Al.w[b] : d->Al.b[b];
                if(!bno)
                    break;

                Q_ASSERT(bno >= datablk() && bno <= blkmax());

                QMap<quint16, char*>::iterator it = _cache->find(bno);
                if( it != _cache->end())
                {
                    result = writeabs(bno, 0, it.value(), blksz());
                    delete [] it.value();
                    _cache->remove(bno);
                }
            }
        }
    }
    return result;
}

bool CPMDrive::writeabs(quint16 block, quint16 offs, const char *data, quint16 sz)
{
    QMutexLocker locker(&_mutex);
    bool result = isOpen();
    Q_ASSERT( block <= blkmax());
    Q_ASSERT((offs + sz) <= blksz());

    if(result)
    {
        _log << "WABS: b/o/s " <<  block << '/' << offs << '/' << sz << endl;

        QMap<quint16, char*>::iterator it = _cache->find(block);
        QString hostfile;

        qint32 blk = findBlock(hostfile,block);
        if(blk != -1)
        {
            watch(false);
            QFile f(_path + '/' + hostfile);
            Q_ASSERT(f.open(QIODevice::WriteOnly));
            result = f.seek(blk+offs) && f.write(data,sz) == sz;
            f.close();
            watch(true);
        }
        else
        {
            if(it == _cache->end() || !it.value())
            {
                (*_cache)[block] = new char[blksz()]();
                Q_ASSERT((*_cache)[block]);
            }
            memcpy((*_cache)[block] + offs, data, sz);
            result = true;
        }
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

    quint16 block = getblk(track, xlt(sectr));
    quint16 offst = getoff(track, xlt(sectr));

    Q_ASSERT( track < tracks());
    Q_ASSERT( sectr > 0 &&  sectr <= _dpb.spt);

    _log << "WCPM: t/s " <<  track << '/' << sectr << endl;

    if(track < _dpb.off)
        result = writesys( track, sectr, data);
    else if( block < datablk())
        result = writedir( block, offst, data, secsz());
    else
        result = writeabs(block, offst, data, secsz());

    return result;
}


quint16 CPMDrive::xlt(quint16 s) const
{
    Q_ASSERT(s > 0 && s <= _dpb.spt);

    return _mode & LINEAR ? (s-1) : _xlt[s-1];
}
