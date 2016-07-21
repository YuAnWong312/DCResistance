#include "zhizu.h"
#include "ui_zhizu.h"

Zhizu::Zhizu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Zhizu)
{
    ui->setupUi(this);
}

Zhizu::~Zhizu()
{
    delete ui;
}
