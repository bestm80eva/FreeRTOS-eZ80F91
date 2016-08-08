#ifndef CDRIVEGUI_H
#define CDRIVEGUI_H

#include <QDockWidget>

namespace Ui {
class CDriveGUI;
}

class CDriveGUI : public QDockWidget
{
    Q_OBJECT

public:
    explicit CDriveGUI(QWidget *parent = 0);
    ~CDriveGUI();

private:
    Ui::CDriveGUI *ui;
};

#endif // CDRIVEGUI_H
