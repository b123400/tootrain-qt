#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QCoreApplication>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->versionLabel->setText("v" + QCoreApplication::applicationVersion());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
