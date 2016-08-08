#include "clientgui.h"
#include "ui_clientgui.h"

ClientGUI::ClientGUI(QWidget *parent) :
    QDockWidget(parent),
    _client(0),
    ui(new Ui::ClientGUI)

{
    ui->setupUi(this);
}

ClientGUI::~ClientGUI()
{
    delete ui;
}

void ClientGUI::paintEvent(QPaintEvent *event)
{
    if(_client)
    {
        ui->IPEdit->setText(_client->host());
        ui->requests->setText(QString("%1").arg(_client->requests()));
        ui->req->setText(QString("%1").arg(_client->indata()));
        ui->rsp->setText(QString("%1").arg(_client->outdata()));
    }
    QDockWidget::paintEvent(event);
}

CPMClient *ClientGUI::setClient(CPMClient *client)
{
    CPMClient *res = _client;
    _client = client;
    update();
    return res;
}
