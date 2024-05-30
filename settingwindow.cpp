#include <QNetworkRequest>
#include <QUrl>

#include "settingwindow.h"
#include "ui_settingwindow.h"
#include "settingmanager.h"
#include "mastodon/client.h"

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    connect(ui->loginButton, &QAbstractButton::clicked, this, &SettingWindow::loginButtonClicked);
    connect(ui->testButton, &QAbstractButton::clicked, this, &SettingWindow::testProfile);
    loadAccount();
}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::loadAccount() {
    auto accounts = SettingManager::shared().getAccounts();
    if (accounts.size()) {
        currentAccount = accounts.at(0);
        ui->currentAccountName->setText("Account: " + currentAccount->username);
        ui->loginButton->setText("Logout");
        // We only take the first one, and discard to rest
        currentAccount->setParent(this);
        // for (int i = 1; i < accounts.size(); i++) {
        //     delete accounts.at(i);
        // }
    } else {
        currentAccount = nullptr;
        ui->currentAccountName->setText("Not logged in");
        ui->loginButton->setText("Login");
    }
}

void SettingWindow::loginButtonClicked() {
    if (currentAccount) {
        delete currentAccount;
        SettingManager::shared().clearAccounts();
        loadAccount();
    } else {
        this->mastodonOAuthWindow = new MastodonOauthWindow(this);
        connect(this->mastodonOAuthWindow, &QDialog::finished, this, &SettingWindow::mastodonAccountFinished);
        connect(this->mastodonOAuthWindow, &MastodonOauthWindow::authenticated, this, &SettingWindow::mastodonAccountAuthenticated);
        this->mastodonOAuthWindow->setWindowModality(Qt::WindowModality::WindowModal);
        this->mastodonOAuthWindow->show();
    }
}

void SettingWindow::mastodonAccountFinished() {
    delete mastodonOAuthWindow;
}

void SettingWindow::mastodonAccountAuthenticated(MastodonAccount *account) {
    qDebug() << account->username;
    QList<Account*> list;
    list.append(account);
    SettingManager::shared().saveAccounts(list);
    loadAccount();
}

void SettingWindow::testProfile() {
    if (!currentAccount) return;
    MastodonAccount *ma = (MastodonAccount*)currentAccount;
    MastodonClient::shared().verifyCredentials(ma->app, [=](MastodonAccount* account){
        qDebug() << "OK:" << account->username;
    });
}
