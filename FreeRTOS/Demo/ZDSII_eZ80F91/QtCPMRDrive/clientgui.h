#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include "cpmclient.h"

#include <QDockWidget>
#include <QPaintEvent>

namespace Ui {
class ClientGUI;
}

class ClientGUI : public QDockWidget
{
    Q_OBJECT

public:
    explicit ClientGUI(QWidget *parent = 0);
    ~ClientGUI();

    CPMClient *setClient(CPMClient *client);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    CPMClient *_client;
    Ui::ClientGUI *ui;
};

#endif // CLIENTGUI_H
