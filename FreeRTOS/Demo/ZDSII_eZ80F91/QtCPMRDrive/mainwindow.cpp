
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
       << ", drv " << dec << (unsigned) pdu.devid << ")";
    return ots;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _log(stdout),
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
        _log << "Req:" << *request << "= " << res << ", Res:" << *response << endl;

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
