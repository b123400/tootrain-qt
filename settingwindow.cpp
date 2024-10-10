#include <QNetworkRequest>
#include <QUrl>
#include <QGuiApplication>
#include <QMessageBox>
#include <QDir>

#include "settingwindow.h"
#include "QtGui/qscreen.h"
#include "ui_settingwindow.h"
#include "settingmanager.h"
#include "mastodon/client.h"

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    connect(ui->loginButton, &QAbstractButton::clicked, this, &SettingWindow::loginButtonClicked);
    connect(ui->configButton, &QAbstractButton::clicked, this, &SettingWindow::configureButtonClicked);
    connect(ui->testButton, &QAbstractButton::clicked, this, &SettingWindow::testProfile);
    connect(ui->checkOrUpdateButton, &QAbstractButton::clicked, this, &SettingWindow::checkOrUpdateClicked);
    connect(&checkProcess, &QProcess::finished, this, &SettingWindow::checkFinished);
    connect(&checkProcess, &QProcess::errorOccurred, this, &SettingWindow::updateCheckErrored);
    connect(&updateProcess, &QProcess::finished, this, &SettingWindow::updateFinished);
    connect(&updateProcess, &QProcess::errorOccurred, this, &SettingWindow::updateCheckErrored);
    loadAccount();
    loadScreens();

    connect(ui->screenComboBox, &QComboBox::currentIndexChanged, this, &SettingWindow::screenIndexChanged);

    checkForUpdate();

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
        ui->currentAccountName->setText(tr("Account: ") + currentAccount->fullUsername());
        ui->loginButton->setText(tr("Logout"));
        ui->configButton->show();
        currentAccount->setParent(this);
    } else {
        currentAccount = nullptr;
        ui->currentAccountName->setText(tr("Not logged in"));
        ui->loginButton->setText(tr("Login"));
        ui->configButton->hide();
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
        currentAccount = nullptr;
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
    mastodonOAuthWindow = nullptr;
}

void SettingWindow::mastodonAccountAuthenticated(MastodonAccount *account) {
    qDebug() << account->username;
    // TODO: Support multiple accounts
    QList<Account*> list;
    list.append(account);
    SettingManager::shared().saveAccounts(list);
    loadAccount();
}

void SettingWindow::configureButtonClicked() {
    if (this->mastodonSettingWindow != nullptr) {
        this->mastodonSettingWindow->show();
    } else {
        MastodonSettingWindow *settingWindow = new MastodonSettingWindow((MastodonAccount*)currentAccount, this);
        this->mastodonSettingWindow = settingWindow;
        connect(this->mastodonSettingWindow, &QDialog::finished, this, &SettingWindow::mastodonSettingFinished);
        connect(this->mastodonSettingWindow, &MastodonSettingWindow::accountUpdated, this, &SettingWindow::mastodonSettingUpdated);
        this->mastodonSettingWindow->setWindowModality(Qt::WindowModality::WindowModal);
        this->mastodonSettingWindow->show();
    }
}

void SettingWindow::mastodonSettingFinished() {
    delete mastodonSettingWindow;
    mastodonSettingWindow = nullptr;
}

void SettingWindow::mastodonSettingUpdated(MastodonAccount *account) {
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
        delete account;
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

QString SettingWindow::maintenanceToolPath() {
    QString maintenanceToolPath =
#if defined(Q_OS_WIN)
        "../maintenancetool.exe";
#elif defined(Q_OS_MAC)
        "../../../maintenancetool.app/Contents/MacOS/maintenancetool";
#else
        "../maintenancetool";
#endif
    return maintenanceToolPath;
}

void SettingWindow::checkOrUpdateClicked() {
    if (hasNewVersion) {
        runUpdate();
    } else {
        checkForUpdate();
    }
}

void SettingWindow::checkForUpdate() {
    if (hasNewVersion) return;
    ui->updateStatusLabel->setText(tr("Checking for update..."));
    ui->checkOrUpdateButton->setEnabled(false);
    QString maintenanceToolPath = this->maintenanceToolPath();
    QString myPath = QCoreApplication::applicationDirPath();
    QDir myDir = QDir(myPath);
    QString absPath = QDir::cleanPath(myDir.absoluteFilePath(maintenanceToolPath));
    qDebug() << "Update binary path: " << absPath;
    checkProcess.start(absPath, {"check-updates"});
}

void SettingWindow::checkFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "checkfinished exitCode=" << exitCode << ", exitStatus=" << exitStatus;
    QByteArray stdOut = checkProcess.readAllStandardOutput();
    qDebug() << "checkfinished>" << stdOut;
    hasNewVersion = stdOut.contains("<updates>");
    ui->checkOrUpdateButton->setEnabled(true);

    if (hasNewVersion) {
        ui->updateStatusLabel->setText(tr("New version available"));
        ui->checkOrUpdateButton->setText(tr("Click here to update"));
    } else {
        ui->updateStatusLabel->setText(tr("No update available"));
    }
    // QMessageBox msgBox;
    // msgBox.setText("Status: " + QString::number(exitCode));
    // msgBox.setDetailedText(stdOut);
    // msgBox.exec();
}

void SettingWindow::updateCheckErrored(QProcess::ProcessError error) {
    QMessageBox msgBox;

    switch (error) {
    case QProcess::ProcessError::ReadError:
        msgBox.setText("ReadError");
        break;
    case QProcess::ProcessError::UnknownError:
        msgBox.setText("UnknownError");
        break;
    case QProcess::ProcessError::WriteError:
        msgBox.setText("WriteError");
        break;
    case QProcess::ProcessError::Crashed:
        msgBox.setText("Crashed");
        break;
    case QProcess::ProcessError::FailedToStart:
        msgBox.setText("FailedToStart");
        break;
    case QProcess::ProcessError::Timedout:
        msgBox.setText("Timedout");
        break;
    }
    msgBox.exec();
    ui->checkOrUpdateButton->setEnabled(true);
}

void SettingWindow::runUpdate() {
    if (!hasNewVersion) return;
    ui->updateStatusLabel->setText(tr("Updating..."));
    ui->checkOrUpdateButton->setEnabled(false);
    QString maintenanceToolPath = this->maintenanceToolPath();
    QString myPath = QCoreApplication::applicationDirPath();
    QDir myDir = QDir(myPath);
    QString absPath = QDir::cleanPath(myDir.absoluteFilePath(maintenanceToolPath));
    qDebug() << "Update binary path: " << absPath;
    updateProcess.start(absPath, {"update", "-c"});
}

void SettingWindow::updateFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    ui->updateStatusLabel->setText(tr("Restarting..."));
    qint64 pid;
    auto started = QProcess::startDetached(QCoreApplication::applicationFilePath(), {}, "", &pid);
    qDebug() << "started:" << started << " pid:" << pid;
    QCoreApplication::exit(0);
}
