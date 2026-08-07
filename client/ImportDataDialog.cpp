#include "ImportDataDialog.h"
#include "ui_ImportDataDialog.h"
#include "CFramelessWindowBase.h"
ImportDataDialog::ImportDataDialog(QWidget *parent) :
    CDialogBase(parent),
    ui(new Ui::ImportDataDialog)
{
    ui->setupUi(CDialogBase::contentsWidget());
    initSize();
    setWindowTitleText("Import");
    setWindowTitleIcon(QIcon(":/Resource/Resource/Icon/通信.png"));
}

ImportDataDialog::~ImportDataDialog()
{
    delete ui;
}
