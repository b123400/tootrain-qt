#include "settingwindow.h"
#include "ui_settingwindow.h"

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    connect(ui->addAccountButton, &QAbstractButton::clicked, this, &SettingWindow::addAccountButtonClicked);
    connect(ui->deleteAccountButton, &QAbstractButton::clicked, this, &SettingWindow::deleteAccountButtonClicked);
}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::addAccountButtonClicked() {
    this->mastodonOAuthWindow = new MastodonOauthWindow(this);
    connect(this->mastodonOAuthWindow, &QDialog::finished, this, &SettingWindow::mastodonAccountFinished);
    connect(this->mastodonOAuthWindow, &MastodonOauthWindow::authenticated, this, &SettingWindow::mastodonAccountAuthenticated);
    this->mastodonOAuthWindow->setWindowModality(Qt::WindowModality::WindowModal);
    this->mastodonOAuthWindow->show();
}

void SettingWindow::deleteAccountButtonClicked() {

}

void SettingWindow::mastodonAccountFinished() {
    delete mastodonOAuthWindow;
}

void SettingWindow::mastodonAccountAuthenticated(MastodonAccount *account) {
    qDebug() << account->username;
}
