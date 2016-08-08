#include "cpmclient.h"

CPMClient::CPMClient(const QString &host, QWidget *parent)
    : QWidget(parent)
    ,_requests(0)
    ,_reqdata(0)
    ,_rspdata(0)
    ,_host(host)
{

}

bool CPMClient::req(const hdr_t &request, QByteArray &resp)
{
    bool result = false;

    _requests++;
    _reqdata += request.pdusz;

    CPMDrive *drive = 0;
    hdr_t    *response = 0;

    resp.resize(sizeof(hdr_t));
    response = (hdr_t*) resp.data();
    memcpy(response, &request, sizeof(hdr_t));
    response->cmdid |= RDSK_Response;

    QMap< quint8, CPMDrive*>::iterator it = _drive.find(request.devid);

    if(it != _drive.end())
        drive = it.value();

    switch((pdutype_t)request.cmdid)
    {
        case RDSK_MountRequest:
        {
            mountreq_t &mreq = (mountreq_t&) request;
            if(!drive)
            {
                const uint8_t *xlt = mreq.xlt ? &mreq.xlt:0;

                drive = _drive[mreq.hdr.devid] = new CPMDrive((const char*)mreq.diskid,
                                                                 mreq.secsz, mreq.dpb,
                                                                 xlt,
                                                                 (iomode_t)mreq.mode);
                Q_ASSERT(drive);
                emit adddrive(this, drive);
            }


            result = drive->open();
            _seqnz = result ? request.seqnz : 0;
        }
        break;

        case RDSK_UnmountRequest:
        if(drive && request.seqnz == _seqnz)
        {
            result = drive->close();
        }
        break;

        case RDSK_ReadRequest:
        if(drive && request.seqnz == _seqnz)
        {
            ioreq_t &req = (ioreq_t&) request;
            ioreq_t *rsp;
            resp.resize(sizeof(ioreq_t) + drive->secsz());
            response = (hdr_t*) resp.data();
            rsp = (ioreq_t*) resp.data();
            rsp->track = req.track;
            rsp->sect  = req.sect;
            result = drive->read(req.track, req.sect, (char*)&rsp->data);
        }
        break;

        case RDSK_WriteRequest:
        if(drive && request.seqnz == _seqnz)
        {
            ioreq_t &req = (ioreq_t&) request;
            result = drive->write(req.track, req.sect, (const char*)&req.data);
        }
        break;

    default:
        Q_ASSERT(false);
        break;
    }

    response->pdusz  =  resp.size();
    response->seqnz = ~_seqnz;

    if(!result)
    {
        response->cmdid |= RDSK_ErrorFlag;
    }
    else
        _seqnz++;

    _rspdata += response->pdusz;

    return result;
}
