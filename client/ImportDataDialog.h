#ifndef ImportDataDialog_H
#define ImportDataDialog_H

#include "CDialogBase.h"

namespace Ui {
class ImportDataDialog;
}

class ImportDataDialog : public CDialogBase
{
    Q_OBJECT

public:
    explicit ImportDataDialog(QWidget *parent = 0);
    ~ImportDataDialog();

private:
    Ui::ImportDataDialog *ui;
};

#endif // ImportDataDialog_H
