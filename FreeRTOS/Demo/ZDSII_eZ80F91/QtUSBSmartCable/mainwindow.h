#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "smartcableserver.h"

#include <QMainWindow>
#include <QCloseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void listen(bool x);

protected:

private:
    Ui::MainWindow *ui;
    void closeEvent(QCloseEvent *bar);
    void loadSettings();
    void saveSettings();
    SmartCableServer    _tcp;
};

#endif // MAINWINDOW_H
