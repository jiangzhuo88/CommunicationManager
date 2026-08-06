#include "CenterWidget.h"
#include "ui_CenterWidget.h"

CenterWidget::CenterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CenterWidget)
{
    ui->setupUi(this);
}

CenterWidget::~CenterWidget()
{
    delete ui;
}
