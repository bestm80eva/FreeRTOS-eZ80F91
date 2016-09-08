#include "hostdirdrive.h"

#include <QDir>
#include <QDebug>

HostDirDrive::HostDirDrive(const QString &path, quint16 secsz, const dpb_t &dpb, const quint8 *xlt, iomode_t mode, QObject *parent)
  :CPMDrive(path, secsz, dpb, xlt, mode, parent)
  ,_open(false)
{
}

HostDirDrive::~HostDirDrive()
{

}

QString HostDirDrive::getHostname(const dir_t& ext)
{
    QByteArray cpmname((char*)&ext.status,12);
    QString hostname;

    QMap<QByteArray, QString>::const_iterator it = _hostname.find(cpmname);

    if(it == _hostname.end())
    {
        hostname = QString("%1.%2.~%3").arg(cpmname.mid(1,8).trimmed().constData())
                .arg(cpmname.mid(9,3).trimmed().constData())
                .arg((ushort) ext.status, 3,10,QLatin1Char('0'));
        QFile f(name() + '/' + hostname);
        if(!f.exists() && !ext.status)
            hostname = hostname.left(hostname.size()-5);
    }
    else
        hostname = it.value();

    return hostname;

}

dir_t &HostDirDrive::dir(quint16 idx)
{
    Q_ASSERT(idx < maxdir());

    quint16 sec = (idx * sizeof(dir_t)) / secsz();
    quint16 off = (idx * sizeof(dir_t)) % secsz();
    QMap<quint32, char*>::iterator it = _cache.find(sec);

    if(it == _cache.end())
    {
        _cache[sec] = new char [secsz()]();
        it = _cache.find(sec);
    }
    Q_ASSERT(it != _cache.end());

    return *(dir_t*) (it.value() + off);
}

bool HostDirDrive::open()
{
    bool result = isOpen();

    if(!result)
    {
        QMutexLocker locker(&_mutex);
        QDir hdir(name());
        result = hdir.exists();
        if(result)
        {
            hdir.setFilter(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot|QDir::Readable|QDir::Writable);
            QFileInfoList list = hdir.entryInfoList();

            quint16 i;
            quint16 block = datablk();
            quint16 diridx=0;

            for(i = 0; i < maxdir(); i++)
                dir(i).status = UNUSED;

            for (i = 0; i < list.size() &&      // for each file on host-dir
                 block < maxblk() &&            // at least one block free
                 diridx < maxdir(); ++i)        // at least one extend free
            {
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
                    // extend setup
                    memcpy(&dir(diridx).name,cpmname.toLocal8Bit().data(),8);
                    memcpy(&dir(diridx).next,cpmext.toLocal8Bit().data(),3);
                    dir(diridx).status = cpmuser;

                    memset(&dir(diridx).Xl, 0, sizeof(dir_t)-12);
                    dir(diridx).Xl = (extend & 0x1F);
                    dir(diridx).Xh = (extend >> 5) & 0x3F;

                    qint64 extbytes = fi.size() - offset;   // bytes left ...
                    if(extbytes > maxext())
                        extbytes = maxext();                 // on this extend

                    if(!extend)                             // add hostname map
                    {
                        QByteArray key((const char*)&dir(diridx).status,12);
                        _hostname[key] = fulname;
                    }

                    if(extbytes)
                    {
                        qint16 x = (extbytes + secsz() -1) / secsz();

                        Q_ASSERT(x <= 0x80);
                        dir(diridx).Rc = x;
                        dir(diridx).Bc = 0; // CP/M 2.2

                        // Assign blocks to extend
                        for(int i = 0; i < extcnt() && extbytes > 0; i++, block++)
                        {
                            if(extcnt() == 8)
                                dir(diridx).Al.w[i] = block;
                            else
                                dir(diridx).Al.b[i] = block;

                            extbytes -= blksz();
                            offset += blksz();
                        }
                    }

                    log() << dir(diridx) << endl;
                    diridx++;
                    extend++;

                } while(offset < fi.size() && diridx < maxdir());
                file.close();
            }
            _open = result = true;
        }
    }
    return result;
}


bool HostDirDrive::close()
{
    QMutexLocker locker(&_mutex);
    bool result = _open;
    if(isOpen())
    {
        _hostname.clear();
        _open = false;
    }
    return result;
}

bool HostDirDrive::read(quint16 track, quint16 sect, char* data)
{
    Q_ASSERT(track < maxtrk() && sect << maxsec() && data);
    quint32 as;
    QMutexLocker locker(&_mutex);
    bool result = isOpen();
    log() << "R trk=" << track << ", sec= " << sect << ", xlt= " << xlt(sect) << flush;

    if(result)
    {
        if(track < maxsys())
        {
            as = (track * maxsec() + sect -1) * secsz();
            log() << ", sys= " << as << flush;
            result = readsys(as, data);
        }
        else
        {
            as = abssec(track, xlt(sect));
            log() << ", asec= " << as << flush;
            result = readsec(as, data);
        }
    }
    log() << ", res" << result << endl;
    return result;
}

bool HostDirDrive::write(quint16 track, quint16 sect, const char* data)
{
    Q_ASSERT(track < maxtrk() && sect << maxsec() && data);
    quint32 as;
    QMutexLocker locker(&_mutex);
    bool result = isOpen();
    log() << "W trk=" << track << ", sec= " << sect << ", xlt= " << xlt(sect) << flush;

    if(result)
    {
        if(track < maxsys())
        {
            as = (track * maxsec() + sect -1) * secsz();
            log() << ", sys= " << as << flush;
            result = writesys(as, data);
        }
        else
        {
            as = abssec(track, xlt(sect));
            log() << ", asec= " << as << flush;
            if(isDir(as))
                result = writedir(as, (dir_t*)data);
            else
                result = writesec(as, data);
        }
    }
    log() << ", res" << result << endl;
    return result;
}


bool HostDirDrive::readsec(quint32 abssec, char* data)
{
    QMap<quint32, char*>::iterator it = _cache.find(abssec);
    bool result = it != _cache.end();
    QString hostname;
    if(result)
    {
        log() << ", CACHE" << flush;
        memcpy(data, it.value(), secsz());
    }
    else
    {
        qint32  offset = findBlock(getBlock(abssec), hostname);
        if(offset >= 0)
        {
            offset += secidx(abssec) * secsz();
            log() << ", hostf=" << hostname << ", offset=" << offset << flush;
            QFile f(name() + '/' + hostname);
            result = f.exists();
            if(result && (result = f.open(QIODevice::ReadOnly)))
            {
                if((result = f.seek(offset)))
                {
                    qint32 w = f.read(data,secsz());
                    result = w >= 0;
                    if(result && w < secsz())
                        memset(data+w, 0x1A,secsz()-w);
                }
            }
        }
        else
        {
            memset(data,0xFF,secsz());
            result = true;
        }
    }
    return result;
}

bool HostDirDrive::readsys(quint16 offset, char *data)
{
    log() << ", sysoff=" << offset << endl;
    QFile sys( name() + "/.cpm.sys");
    bool result = sys.open(QIODevice::ReadOnly);
    if(result)
        result = sys.seek(offset) && sys.read(data,secsz()) != -1;
    sys.close();
    if(!result)
    {
        memset(data,0xFF,secsz());
        result = true;
    }
    return result;
}

bool HostDirDrive::writedir(quint32 abssec, const dir_t *data)
{
    log() << ", dir " << secsz() / sizeof(dir_t) * abssec << ", ..." << flush;
    dir_t *odata = (dir_t*) new char[secsz()]();
    QMap<quint32, char*>::iterator it = _cache.find(abssec);
    Q_ASSERT(it != _cache.end() && odata);

    memcpy(odata, it.value(), secsz());
    bool result = writesec(abssec,(const char*)data);

    // for each dir on sector.
    for(unsigned i = 0; result && i < (secsz() / sizeof(dir_t)); i++)
    {
        const dir_t nd = data[i];
        const dir_t od = odata[i];

        if(!((nd.Xh << 5) | nd.Xl))   // first extend
        {
            if( nd.status != UNUSED)       // new dir is active
            {
                if(od.status == UNUSED)   // old dir is free => new file
                {
                    QString hostfile = getHostname(nd);
                    QFile f(name() + '/' + hostfile);
                    result = f.open(QIODevice::ReadWrite);
                    if(result)
                    {
                        QByteArray key((const char*)&nd.status,12);
                        _hostname[key] = hostfile;
                    }
                }
                else if(strncmp(nd.name, od.name,11)) // name has changed
                {
                    QString newname = getHostname(nd);    // new name
                    QString oldname = getHostname(od);   // old name
                    QFile f(name() + '/' + oldname);
                    result = f.rename(name() + '/' + newname);
                    if(result)
                    {
                        QByteArray okey((const char*)&od.status,12);
                        QByteArray nkey((const char*)&nd.status,12);
                        _hostname.remove(okey);
                        _hostname[nkey] = newname;

                    }
                }
            }
            else if(od.status != UNUSED)  // old dir is activ delete
            {
                QString hostfile = getHostname(od);
                QString backup   = hostfile + ".bak";
                QFile b(name() + '/' + backup);
                QFile f(name() + '/' + hostfile);
                b.remove();
                result = f.rename(name() + '/' + backup);
                if(result)
                {
                    QByteArray key((const char*)&od.status,12);
                    _hostname.remove(key);
                }

            }
        }
        if(result && nd.status != UNUSED)  // write cached sectors if any
        {
            for( unsigned b = 0; b < extcnt(); b++)
            {
                quint32 m = (extcnt() == 8 ? nd.Al.w[b] : nd.Al.b[b]) << bshift();
                if(!m)
                    break;
                for( int s = 0; s < (1 << bshift()); s++)
                {
                    QMap<quint32, char*>::iterator it = _cache.find(m+s);
                    if(it != _cache.end())
                    {
                        result = writesec(m+s, it.value());
                        delete [] it.value();
                        _cache.erase(it);
                    }
                }
            }
        }
    }
    delete [] (const char*)odata;
    return result;
}

bool HostDirDrive::writesec(quint32 abssec, const char *data)
{
     bool result = false;
     QString hostname;
     quint16 block  = getBlock(abssec);
     qint32  offset = findBlock(block, hostname);

     if(offset >= 0)
     {
         offset += secidx(abssec) * secsz();
         log() << ", hostf=" << hostname << ", offset=" << offset << flush;
         QFile f(name() + '/' + hostname);
         result = f.exists();
         if(result && (result = f.open(QIODevice::ReadWrite)))
         {
             if((result = f.seek(offset)))
                 result = f.write(data,secsz()) == secsz();
             f.close();
         }
     }
     else
     {
         log() << ", CACHE" << flush;
         QMap<quint32, char*>::iterator it = _cache.find(abssec);
         if(it == _cache.end())
         {
             _cache[abssec] = new char[secsz()]();
             it = _cache.find(abssec);
         }
         result = it != _cache.end();
         if(result)
            memcpy(it.value(), data, secsz());
     }
     return result;
}

bool HostDirDrive::writesys(quint16 offset, const char *data)
{
    log() << "WSYS: off=" << offset << endl;
    QFile sys( name() + "/.cpm.sys");
    bool result = sys.open(QIODevice::WriteOnly);

    if(result)
        result = sys.seek(offset) && sys.write(data,secsz()) == secsz();
    sys.close();
    return result;
}

qint64 HostDirDrive::findBlock(quint16 block, QString& hostname)
{
    qint64 offset = -1;
    unsigned n,b;

    if(block)
        for(n = 0; n < maxdir() && offset == -1; n++)
        {
            if(dir(n).status == UNUSED)
                continue;
            for(b = 0; b < extcnt(); b++)
            {
                quint16 bno = extcnt() == 16 ? dir(n).Al.b[b] : dir(n).Al.w[b];
                if( bno == block)
                {
                    hostname = getHostname(dir(n));
                    offset = ((dir(n).Xl | dir(n).Xh << 5) * extcnt() + b) * blksz();
                    log() << ", found " << dir(n) << endl;
                    log() << "blk " << block << ", hn " << hostname << ", o " << offset << flush;
                    break;
                }
            }
        }
    return offset;
}
