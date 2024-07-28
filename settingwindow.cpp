#include <QNetworkRequest>
#include <QUrl>

#include "settingwindow.h"
#include "QtGui/qscreen.h"
#include "ui_settingwindow.h"
#include "settingmanager.h"
#include "mastodon/client.h"
#include <QGuiApplication>

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    connect(ui->loginButton, &QAbstractButton::clicked, this, &SettingWindow::loginButtonClicked);
    connect(ui->testButton, &QAbstractButton::clicked, this, &SettingWindow::testProfile);
    loadAccount();
    loadScreens();

    connect(ui->screenComboBox, &QComboBox::currentIndexChanged, this, &SettingWindow::screenIndexChanged);

    // Only for debug
    ui->testButton->hide();
}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::loadAccount() {
    auto accounts = SettingManager::shared().getAccounts();
    if (accounts.size()) {
        currentAccount = accounts.at(0);
        ui->currentAccountName->setText("Account: " + currentAccount->fullUsername());
        ui->loginButton->setText("Logout");
        currentAccount->setParent(this);
    } else {
        currentAccount = nullptr;
        ui->currentAccountName->setText("Not logged in");
        ui->loginButton->setText("Login");
    }
}

void SettingWindow::loadScreens() {
    QScreen *selectedScreen = SettingManager::shared().getScreen();
    ui->screenComboBox->clear();
    auto screens = QGuiApplication::screens();
    int i = 0;
    int currentIndex = -1;
    for (auto screen : screens) {
        auto geometry = screen->geometry();
        ui->screenComboBox->addItem(
            screen->name() + " " + QString::number(geometry.width()) + "x" + QString::number(geometry.height()),
            i
            );
        if (screen == selectedScreen) {
            currentIndex = i;
        }
        i++;
    }
    if (currentIndex >= 0) {
        ui->screenComboBox->setCurrentIndex(currentIndex);
    } else {
        ui->screenComboBox->setCurrentIndex(0);
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
    // TODO: Support multiple accounts
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

void SettingWindow::screenIndexChanged(int index) {
    bool getIndexOk = false;
    int screenNumber = ui->screenComboBox->itemData(index).toInt(&getIndexOk);
    if (getIndexOk) {
        auto screens = QGuiApplication::screens();
        SettingManager::shared().setScreen(screens[screenNumber]);
    }
}
