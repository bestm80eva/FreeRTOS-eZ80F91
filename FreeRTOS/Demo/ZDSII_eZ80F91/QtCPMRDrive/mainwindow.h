#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cpmclient.h"
#include "clientgui.h"
#include "cdrivegui.h"

#include <QMainWindow>
#include <QUdpSocket>
#include <QTextStream>
#include <QTreeWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    friend QTextStream& operator << (QTextStream& ots, const hdr_t &pdu);

signals:

public slots:
    void adddrive(CPMClient* client, CPMDrive* drive);
    void deldrive(CPMClient* client, CPMDrive* drive);

protected:
    void selectDrive(CPMDrive* drive);
    void selectClient(CPMClient* client);

private slots:
    void processDatagrams();
    void itemClicked(QTreeWidgetItem *item, int column);
    void mountA();
private:
    Ui::MainWindow *ui;
    QTextStream _log;
    QUdpSocket  _udpSocket;
    ClientGUI   *_clientwidget;
    CDriveGUI   *_drivewidget;
};


#endif // MAINWINDOW_H
