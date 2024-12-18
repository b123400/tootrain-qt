#include <QNetworkRequest>
#include <QUrl>
#include <QGuiApplication>
#include <QMessageBox>
#include <QDir>
#include <QMenu>
#include <QColorDialog>
#include <QFontDialog>

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
    this->setFixedSize(size());

    connect(ui->loginButton, &QAbstractButton::clicked, this, &SettingWindow::loginButtonClicked);
    connect(ui->configButton, &QAbstractButton::clicked, this, &SettingWindow::configureButtonClicked);
    connect(ui->checkOrUpdateButton, &QAbstractButton::clicked, this, &SettingWindow::checkOrUpdateClicked);
    connect(&updateProcess, &QProcess::finished, this, &SettingWindow::updateFinished);
    connect(&updateProcess, &QProcess::errorOccurred, this, &SettingWindow::updateErrored);

    reloadUIFromSettings();

    connect(ui->showAvatarCheckBox, &QCheckBox::checkStateChanged, this, &SettingWindow::showAvatarCheckBoxChanged);
    connect(ui->textColorButton, &QAbstractButton::clicked, this, &SettingWindow::textColorButtonClicked);
    connect(ui->shadowColorButton, &QAbstractButton::clicked, this, &SettingWindow::shadowColorButtonClicked);
    connect(ui->fontButton, &QAbstractButton::clicked, this, &SettingWindow::fontButtonClicked);
    connect(ui->lengthLimitCheckBox, &QCheckBox::checkStateChanged, this, &SettingWindow::textLengthLimitCheckBoxChanged);
    connect(ui->lengthLimitSpinBox, &QSpinBox::valueChanged, this, &SettingWindow::textLengthLimitSpinnerChanged);
    connect(ui->speedSlider, &QSlider::valueChanged, this, &SettingWindow::speedSliderChanged);
    connect(ui->opacitySlider, &QSlider::valueChanged, this, &SettingWindow::opacitySliderChanged);
    connect(ui->hideUrlCheckbox, &QCheckBox::checkStateChanged, this, &SettingWindow::hideUrlCheckboxChanged);
    connect(ui->ignoreContentWarningCheckBox, &QCheckBox::checkStateChanged, this, &SettingWindow::ignoreContentWarningCheckboxChanged);

    connect(ui->screenComboBox, &QComboBox::currentIndexChanged, this, &SettingWindow::screenIndexChanged);

    checkForUpdate();
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

void SettingWindow::reloadUIFromSettings() {
    disconnect(ui->screenComboBox, &QComboBox::currentIndexChanged, this, &SettingWindow::screenIndexChanged);

    loadAccount();
    loadScreens();

    ui->showAvatarCheckBox->setCheckState(
        SettingManager::shared().showUserAvatar() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked
        );
    QPalette textPalette;
    textPalette.setColor(QPalette::Window, SettingManager::shared().textColor());
    ui->textColorFrame->setAutoFillBackground(true);
    ui->textColorFrame->setPalette(textPalette);

    QPalette shadowPalette;
    shadowPalette.setColor(QPalette::Window, SettingManager::shared().shadowColor());
    ui->shadowColorFrame->setAutoFillBackground(true);
    ui->shadowColorFrame->setPalette(shadowPalette);

    QFont font = SettingManager::shared().font();
    ui->fontLabel->setText(tr("Font:") + " " + font.family() + " " + QString::number(font.pointSize()) + "px");

    int textLengthLimit = SettingManager::shared().textLengthLimit();
    if (textLengthLimit <= 0) {
        ui->lengthLimitCheckBox->setCheckState(Qt::CheckState::Unchecked);
        ui->lengthLimitSpinBox->setEnabled(false);
    } else {
        ui->lengthLimitCheckBox->setCheckState(Qt::CheckState::Checked);
        ui->lengthLimitSpinBox->setValue(textLengthLimit);
        ui->lengthLimitSpinBox->setEnabled(true);
    }

    ui->speedSlider->setValue(20-SettingManager::shared().duration());
    ui->opacitySlider->setValue(SettingManager::shared().opacity() * 100);
    ui->hideUrlCheckbox->setCheckState(SettingManager::shared().hideUrl() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->ignoreContentWarningCheckBox->setCheckState(SettingManager::shared().ignoreContentWarning() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    connect(ui->screenComboBox, &QComboBox::currentIndexChanged, this, &SettingWindow::screenIndexChanged);
}

void SettingWindow::loginButtonClicked() {
    if (currentAccount) {
        delete currentAccount;
        currentAccount = nullptr;
        SettingManager::shared().clearAccounts();
        loadAccount();
    } else {
        QMenu menu = QMenu(this);
        QAction mastodonAction = QAction("Mastodon", this);
        connect(&mastodonAction, &QAction::triggered, this, &SettingWindow::loginToMastodon);
        menu.addAction(&mastodonAction);
        QAction misskeyAction = QAction("Misskey", this);
        connect(&misskeyAction, &QAction::triggered, this, &SettingWindow::loginToMisskey);
        menu.addAction(&misskeyAction);
        menu.exec(ui->loginButton->cursor().pos());
    }
}

void SettingWindow::loginToMastodon(bool _checked) {
    this->mastodonOAuthWindow = new MastodonOauthWindow(this);
    connect(this->mastodonOAuthWindow, &QDialog::finished, this, &SettingWindow::mastodonAccountFinished);
    connect(this->mastodonOAuthWindow, &MastodonOauthWindow::authenticated, this, &SettingWindow::mastodonAccountAuthenticated);
    this->mastodonOAuthWindow->setWindowModality(Qt::WindowModality::WindowModal);
    this->mastodonOAuthWindow->show();
}

void SettingWindow::loginToMisskey(bool _checked) {
    this->misskeyAuthWindow = new MisskeyAuthWindow(this);
    connect(this->misskeyAuthWindow, &QDialog::finished, this, &SettingWindow::misskeyAccountFinished);
    connect(this->misskeyAuthWindow, &MisskeyAuthWindow::authenticated, this, &SettingWindow::misskeyAccountAuthenticated);
    this->misskeyAuthWindow->setWindowModality(Qt::WindowModality::WindowModal);
    this->misskeyAuthWindow->show();
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
    if (this->currentAccount == nullptr) return;
    if (dynamic_cast<MastodonAccount*>(this->currentAccount) != nullptr) {
        openMastodonSettings();
    } else if (dynamic_cast<MisskeyAccount*>(this->currentAccount) != nullptr) {
        openMisskeySettings();
    }
}

void SettingWindow::openMastodonSettings() {
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
void SettingWindow::openMisskeySettings() {
    if (this->misskeySettingWindow != nullptr) {
        this->misskeySettingWindow->show();
    } else {
        MisskeySettingWindow *settingWindow = new MisskeySettingWindow((MisskeyAccount*)currentAccount, this);
        this->misskeySettingWindow = settingWindow;
        connect(this->misskeySettingWindow, &QDialog::finished, this, &SettingWindow::misskeySettingFinished);
        connect(this->misskeySettingWindow, &MisskeySettingWindow::accountUpdated, this, &SettingWindow::misskeySettingUpdated);
        this->misskeySettingWindow->setWindowModality(Qt::WindowModality::WindowModal);
        this->misskeySettingWindow->show();
    }
}

void SettingWindow::mastodonSettingFinished() {
    delete mastodonSettingWindow;
    mastodonSettingWindow = nullptr;
}

void SettingWindow::misskeySettingFinished() {
    delete misskeySettingWindow;
    misskeySettingWindow = nullptr;
}

void SettingWindow::mastodonSettingUpdated(MastodonAccount *account) {
    // TODO: Support multiple accounts
    QList<Account*> list;
    list.append(account);
    SettingManager::shared().saveAccounts(list);
    loadAccount();
}

void SettingWindow::misskeySettingUpdated(MisskeyAccount *account) {
    // TODO: Support multiple accounts
    QList<Account*> list;
    list.append(account);
    SettingManager::shared().saveAccounts(list);
    loadAccount();
}

void SettingWindow::misskeyAccountFinished() {
    delete misskeyAuthWindow;
    misskeyAuthWindow = nullptr;
}

void SettingWindow::misskeyAccountAuthenticated(MisskeyAccount *account) {
    qDebug() << account->username;
    // TODO: Support multiple accounts
    QList<Account*> list;
    list.append(account);
    SettingManager::shared().saveAccounts(list);
    loadAccount();
}

void SettingWindow::screenIndexChanged(int index) {
    bool getIndexOk = false;
    int screenNumber = ui->screenComboBox->itemData(index).toInt(&getIndexOk);
    if (getIndexOk) {
        auto screens = QGuiApplication::screens();
        SettingManager::shared().setScreen(screens[screenNumber]);
    }
}

void SettingWindow::showAvatarCheckBoxChanged(Qt::CheckState checkState) {
    SettingManager::shared().setShowUserAvatar(checkState == Qt::CheckState::Checked);
}

void SettingWindow::textColorButtonClicked() {
    QColor currentColor = SettingManager::shared().textColor();
    QColor color = QColorDialog::getColor(currentColor, this);
    if (!color.isValid()) return;
    SettingManager::shared().setTextColor(color);
    reloadUIFromSettings();
}

void SettingWindow::shadowColorButtonClicked() {
    QColor currentColor = SettingManager::shared().shadowColor();
    QColor color = QColorDialog::getColor(currentColor, this);
    if (!color.isValid()) return;
    SettingManager::shared().setShadowColor(color);
    reloadUIFromSettings();
}

void SettingWindow::fontButtonClicked() {
    bool ok;
    QFont currentFont = SettingManager::shared().font();
    QFont newFont = QFontDialog::getFont(&ok, currentFont, this);
    if (ok) {
        SettingManager::shared().setFont(newFont);
    }
    reloadUIFromSettings();
}

void SettingWindow::textLengthLimitSpinnerChanged(int value) {
    SettingManager::shared().setTextLengthLimit(value);
    reloadUIFromSettings();
}

void SettingWindow::textLengthLimitCheckBoxChanged(Qt::CheckState checked) {
    if (checked == Qt::CheckState::Unchecked) {
        SettingManager::shared().setTextLengthLimit(-1);
    } else {
        SettingManager::shared().setTextLengthLimit(50);
    }
    reloadUIFromSettings();
}

void SettingWindow::speedSliderChanged(int value) {
    SettingManager::shared().setDuration(20 - value);
}

void SettingWindow::opacitySliderChanged(int value) {
    SettingManager::shared().setOpacity(value / 100.0f);
}

void SettingWindow::hideUrlCheckboxChanged(Qt::CheckState checkState) {
    SettingManager::shared().setHideUrl(checkState == Qt::CheckState::Checked);
}

void SettingWindow::ignoreContentWarningCheckboxChanged(Qt::CheckState checkState) {
    SettingManager::shared().setIgnoreContentWarning(checkState == Qt::CheckState::Checked);
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

    SettingManager::shared().checkForUpdate([=](bool hasUpdate) {
        hasNewVersion = hasUpdate;
        ui->checkOrUpdateButton->setEnabled(true);

        if (hasNewVersion) {
            ui->updateStatusLabel->setText(tr("New version available"));
            ui->checkOrUpdateButton->setText(tr("Click here to update"));
        } else {
            ui->updateStatusLabel->setText(tr("No update available"));
        }
    });
}

void SettingWindow::updateErrored(QProcess::ProcessError error) {
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
    QString maintenanceToolPath = SettingManager::shared().maintenanceToolPath();
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
