#include "cdrivegui.h"
#include "ui_cdrivegui.h"

CDriveGUI::CDriveGUI(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::CDriveGUI)
{
    ui->setupUi(this);
}

CDriveGUI::~CDriveGUI()
{
    delete ui;
}
