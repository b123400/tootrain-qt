#include "misskeysettingwindow.h"
#include "ui_misskeysettingwindow.h"

MisskeySettingWindow::MisskeySettingWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MisskeySettingWindow)
{
    ui->setupUi(this);

    connect(ui->globalTimelineButton, &QAbstractButton::clicked, this, &MisskeySettingWindow::channelButtonClicked);
    connect(ui->hybridTimelineButton, &QAbstractButton::clicked, this, &MisskeySettingWindow::channelButtonClicked);
    connect(ui->localTimelineButton, &QAbstractButton::clicked, this, &MisskeySettingWindow::channelButtonClicked);
    connect(ui->mainTimelineButton, &QAbstractButton::clicked, this, &MisskeySettingWindow::channelButtonClicked);
    connect(ui->homeTimelineButton, &QAbstractButton::clicked, this, &MisskeySettingWindow::channelButtonClicked);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MisskeySettingWindow::accepted);
}

MisskeySettingWindow::~MisskeySettingWindow()
{
    delete ui;
}

void MisskeySettingWindow::channelButtonClicked() {

}

void MisskeySettingWindow::acceptClicked() {

}
