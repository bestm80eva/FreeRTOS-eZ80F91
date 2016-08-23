#include "hostdirdrive.h"

#include <QDir>
#include <QDebug>

HostDirDrive::HostDirDrive(const QString &path, quint16 secsz, const dpb_t &dpb, const quint8 *xlt, iomode_t mode, QObject *parent)
  :CPMDrive(path, secsz, dpb, xlt, mode, parent)
  ,_open(false)
  ,_dir(dpb.drm+1)
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
                .arg((quint16) cpmname[0]);
        QFile f(name() + '/' + hostname);
        if(!f.exists() && !cpmname[0])
            hostname = hostname.left(hostname.size()-3);
    }
    else
        hostname = it.value();

    return hostname;

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

            // init directory
            for(i = 0; i < maxdir(); i++)
            {
                memset(&_dir[i],0,sizeof(dir_t));
                _dir[i].status = UNUSED;
            }

            for (int i = 0; i < list.size() &&  // for each file on host-dir
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

                // extend setup
                dir_t d;
                memcpy(&d.name,cpmname.toLocal8Bit().data(),8);
                memcpy(&d.next,cpmext.toLocal8Bit().data(),3);
                d.status = cpmuser;
                QByteArray key((const char*)&d.status,12);
                _hostname[key] = fulname;


                // fill up all needed extends
                do {
                    memset(&d.Xl, 0, sizeof(dir_t)-12);
                    d.Xl = (extend & 0x1F);
                    d.Xh = (extend >> 5) & 0x3F;

                    qint64 extbytes = fi.size() - offset;   // bytes left ...
                    if(extbytes > maxext())
                        extbytes = maxext();                 // on this extend

                    if(extbytes)
                    {
                        qint16 x = (extbytes + secsz() -1) / secsz();

                        Q_ASSERT(x <= 0x80);
                        d.Rc = x;
                        d.Bc = 0; // CP/M 2.2

                        // Assign blocks to extend
                        for(int i = 0; i < extcnt() && extbytes > 0; i++, block++)
                        {
                            if(extcnt() == 8)
                                d.Al.w[i] = block;
                            else
                                d.Al.b[i] = block;

                            extbytes -= blksz();
                            offset += blksz();
                        }
                    }
                    _dir[diridx++] = d;
                    log() << d << endl;
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
    _open = false;
    return true;
}

bool HostDirDrive::read(quint16 track, quint16 sect, char* data)
{
    bool result = isOpen();
    quint16 block = getBlockNo(track, xlt(sect));
    quint16 didx = secidx(track, xlt(sect));

    log() << "R trk=" << track << ", sec= " << sect << ", blk=" << block << ", soff=" << didx;

    if(result)
    {
        if(track < maxsys())
            result = readsys((track * maxsec() + sect -1) * secsz(), data);
        else
        {
            if( isDir(block))
                result = readdir(block, didx, (dir_t*)data);
            else
                result = readsec(block, didx, data);
        }
    }
    log() << ", res" << result << endl;
    return result;
}

bool HostDirDrive::write(quint16 track, quint16 sect, const char* data)
{
    bool result = isOpen();
    quint16 block = getBlockNo(track, xlt(sect));
    quint16 didx = secidx(track, xlt(sect));

    log() << "W trk=" << track << ", sec= " << sect << ", blk=" << block << ", soff=" << didx;

    if(result)
    {
        if(track < maxsys())
            result = writesys((track * maxsec() + sect -1) * secsz(), data);
        else
        {
            if( isDir(block))
                result = writedir(didx, (dir_t*)data);
            else
                result = writesec(block, didx & blkmsk(), data);
        }
    }

    return result;
}

bool HostDirDrive::readdir(quint16 block, quint16 sec, dir_t* data)
{
    quint16 idx = (block*blksz() + sec * secsz()) / sizeof(dir_t);
    log() << ", dir " << idx;

    for(unsigned i = 0; i < (secsz() / sizeof(dir_t)); i++)
    {
        data[i] = _dir.at(idx+i);
        log() << endl << data[i];
    }
    return true;
}

bool HostDirDrive::readsec(quint16 block, quint16 sec, char* data)
{
    bool result = false;
    QString hostname;
    qint32  offset = findBlock(block, hostname);
    if(offset >= 0)
    {
        offset += sec * secsz();
        log() << ", hostf=" << hostname << ", offset=" << offset;
        QFile f(name() + '/' + hostname);
        result = f.exists();
        if(result && (result = f.open(QIODevice::ReadOnly)))
        {
            if((result = f.seek(offset)))
                result = f.read(data,secsz()) == secsz();
            f.close();
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
    return result;
}

bool HostDirDrive::writedir(quint16 idx, const dir_t* data)
{
    return false;
}

bool HostDirDrive::writesec(quint16 block, quint16 sec, const char *data)
{
    return false;
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
    for(n = 0; n < maxdir() && offset == -1; n++)
    {
        if(_dir.at(n).status == UNUSED)
            continue;
        for(b = 0; b < extcnt(); b++)
        {
            quint16 bno = extcnt() == 16 ? _dir.at(n).Al.b[b] : _dir.at(n).Al.w[b];
            if( bno == block)
            {
                hostname = getHostname(_dir.at(n));
                offset = ((_dir.at(n).Xl | _dir.at(n).Xh << 5) * extcnt() + b) * blksz();
                log() << ", found " << _dir.at(n) << endl;
                log() << "blk " << block << ", hn " << hostname << ", o " << offset;
                break;
            }
        }
    }
    return offset;
}
