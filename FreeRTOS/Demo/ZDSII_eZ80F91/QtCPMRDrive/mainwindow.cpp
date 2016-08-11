
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QHostInfo>
#include <QDebug>

QTextStream& operator << (QTextStream& ots, const hdr_t &pdu)
{
    ots <<  "PDU[" << dec << pdu.seqnz
        << "](sz " << dec << pdu.pdusz
       << ", cmd " << hex << (unsigned) pdu.cmdid
       << ", drv " << dec << (unsigned) pdu.devid << ") ";
    return ots;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _log(stderr),
    _udpSocket(this),
    _clientwidget(new ClientGUI(this)),
    _drivewidget(new CDriveGUI(this))

{
    ui->setupUi(this);
    _clientwidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    _clientwidget->setFeatures(QDockWidget::AllDockWidgetFeatures);

    addDockWidget(Qt::RightDockWidgetArea, _clientwidget);

    _udpSocket.bind(QHostAddress::Any, RDSK_PORT);
    connect(&_udpSocket, SIGNAL(readyRead()), this, SLOT(processDatagrams()));


}

MainWindow::~MainWindow()
{
    delete _clientwidget;
    delete ui;

}

void MainWindow::mountA()
{
    // Default disk IBM 3740 8" 77Trk*26Sec
    static const uint8_t  defxlt[26] = {
         1, 7,13,19,	// sectors  1, 2, 3, 4
        25, 5,11,17,	// sectors  5, 6, 7, 8
        23, 3, 9,15,	// sectors  9,10,11,12
        21, 2, 8,14,	// sectors 13,14,15,16
        20,26, 6,12,	// sectors 17,18,19,20
        18,24, 4,10,	// sectors 21,22,23,24
        16,22			// sectors 25,26
    };

    static const dpb_t defdpb = {
         26,	// sectors per track
          3,	// block shift factor
          7,	// block mask
          0,	// extent mask
        242,	// disk size-1
         63,	// directory max
        192,	// alloc 0
          0,	// alloc 1
         16,	// check size
          2,	// track offset
    };

    QHostAddress sender(QHostAddress::LocalHost);
    quint16 senderPort = RDSK_PORT;


    int sz = sizeof(mountreq_t) + defdpb.spt;
    mountreq_t *req = (mountreq_t*) new uint8_t [sz];

    req->hdr.cmdid = RDSK_MountRequest;
    req->hdr.devid = 0;             // 0=A, 1=B ...
    req->hdr.pdusz = sz;
    req->hdr.seqnz = 1;

    strcpy((char*)&req->diskid[0], "drivea"); // not used jet
    req->mode   = 0;
    req->secsz  = 128;
    memcpy(&req->dpb, &defdpb, sizeof(defdpb));
    memcpy(&req->xlt, &defxlt, defdpb.spt);

    int rsz = _udpSocket.writeDatagram((const char*) req, (qint64)req->hdr.pdusz, sender, senderPort);
    Q_ASSERT(rsz == req->hdr.pdusz);
}

void MainWindow::processDatagrams()
{

    const hdr_t  *request;

    do {
        qint64 sz = _udpSocket.pendingDatagramSize();
        Q_ASSERT(sz >= (qint64) sizeof(hdr_t));
        QByteArray      datagram(sz,0);
        QByteArray      rdatagram;
        QHostAddress    host;
        quint16         port;
        bool            res;

        Q_ASSERT(sz == _udpSocket.readDatagram(datagram.data(), datagram.size(), &host, &port));

        request = (hdr_t*) datagram.constData();
        Q_ASSERT(datagram.size() == request->pdusz);

        // _log << "Req:" << *request << endl;

        // skip respons messages
        if(request->cmdid >= RDSK_Response)
        {
           // _log << ", skip" << endl;
            continue;
        }

        QHostInfo hi = QHostInfo::fromName(host.toString());
        QString   hn(hi.hostName());
        QVariant var;
        CPMClient* client;
        QList<QTreeWidgetItem*> clientitem = ui->treeWidget->findItems(hn,Qt::MatchFixedString);


        if(clientitem.isEmpty())
        {
            client = new CPMClient(hn);
            connect(client,SIGNAL(adddrive(CPMClient*,CPMDrive*)),this,SLOT(adddrive(CPMClient*,CPMDrive*)));
            var.setValue(client);
            QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(hn));
            item->setData(0,Qt::UserRole, var);
            ui->treeWidget->addTopLevelItem(item);
        }
        else
        {
            var = clientitem.first()->data(0,Qt::UserRole);
        }

        client = qobject_cast<CPMClient*>(qvariant_cast<QObject*>(var));
        Q_ASSERT(client);

        res = client->req(*request, rdatagram);
        hdr_t  *response = (hdr_t*)rdatagram.data();
        // _log << "= " << res << ", Res:" << *response << endl;

        Q_ASSERT(res);

        sz = _udpSocket.writeDatagram((const char*)response, (qint64)response->pdusz, host, port);
        if(_clientwidget)
            _clientwidget->update();
        Q_ASSERT( sz == response->pdusz);

    }while(_udpSocket.hasPendingDatagrams());
}

void MainWindow::adddrive(CPMClient* client, CPMDrive* drive)
{
    Q_ASSERT(client);
    Q_ASSERT(drive);

    QVariant var;
    var.setValue(drive);

    QList<QTreeWidgetItem*> clientitem = ui->treeWidget->findItems(client->host(),Qt::MatchFixedString);

    Q_ASSERT(!clientitem.isEmpty());
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(drive->name()));
    item->setData(0,Qt::UserRole, var);
    clientitem.first()->addChild(item);
}

void MainWindow::deldrive(CPMClient* client, CPMDrive* drive)
{
    (void)client;
    (void)drive;
}

void MainWindow::itemClicked(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);

    QVariant var;
    var = item->data(column,Qt::UserRole);

    if(var.canConvert<CPMDrive*>())
        selectDrive(qvariant_cast<CPMDrive*>(var));
    else if(var.canConvert<CPMClient*>())
        selectClient(qvariant_cast<CPMClient*>(var));
}

void MainWindow::selectDrive(CPMDrive* drive)
{
    (void) drive;

}

void MainWindow::selectClient(CPMClient* client)
{
    _clientwidget->setClient(client);

}
