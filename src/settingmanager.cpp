#include "settingmanager.h"
#include <QCoreApplication>
#include <QUrl>
#include <QApplication>
#include <QDir>
#include <QTimer>
#include "mastodon/account.h"
#include "misskey/misskeyaccount.h"

SettingManager::SettingManager(QObject *parent)
    : QObject{parent}
{
    QCoreApplication::setOrganizationName("b123400");
    QCoreApplication::setOrganizationDomain("b123400.net");
    QCoreApplication::setApplicationName("TootRain-qt");
}

void SettingManager::saveAccounts(QList<Account*> accounts) {
    settings.remove("accounts");
    settings.beginWriteArray("accounts");
    for (qsizetype i = 0; i < accounts.size(); i++) {
        settings.setArrayIndex(i);

        Account *a = accounts[i];
        a->saveToSettings(&settings);
    }
    settings.endArray();
    // Assumption: Only 1 account at a time
    // TODO: Support multiple accounts
    emit this->currentAccountChanged();
}

QList<Account*> SettingManager::getAccounts() {
    QList<Account*> accounts;
    int size = settings.beginReadArray("accounts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        Account *account = nullptr;
        QString type = settings.value("type").toString();
        if (type == "mastodon") {
            account = new MastodonAccount(&settings, this);
        } else if (type == "misskey") {
            account = new MisskeyAccount(&settings, this);
        }
        accounts.append(account);
    }
    settings.endArray();
    return accounts;
}

void SettingManager::clearAccounts() {
    settings.remove("accounts");
    emit this->currentAccountChanged();
}

void SettingManager::setScreen(QScreen *screen) {
    auto serialNumber = screen->serialNumber();
    if (!serialNumber.isEmpty()) {
        settings.setValue("screenSerialNumber", serialNumber);
    } else {
        // On some platform QT doesn't support serial number
        auto screens = QGuiApplication::screens();
        int index = screens.indexOf(screen);
        settings.setValue("screenNumber", index);
    }
    emit this->currentScreenChanged();
}

QScreen *SettingManager::getScreen() {
    auto screens = QGuiApplication::screens();
    QString serialNumber = settings.value("screenSerialNumber", "").toString();
    if (!serialNumber.isEmpty()) {
        for (auto screen : screens) {
            if (screen->serialNumber() == serialNumber) {
                return screen;
            }
        }
    }
    bool hasScreenNumber = false;
    int screenNumber = settings.value("screenNumber").toInt(&hasScreenNumber);
    if (hasScreenNumber || screenNumber < screens.length()) {
        return screens[screenNumber];
    }
    return QGuiApplication::primaryScreen();
}

void SettingManager::setShowUserAvatar(bool showUserAvatar) {
    settings.setValue("showUserAvatar", showUserAvatar);
}

bool SettingManager::showUserAvatar() {
    bool showUserAvatar = settings.value("showUserAvatar", true).toBool();
    return showUserAvatar;
}

void SettingManager::setTextColor(QColor color) {
    settings.setValue("textColor", color);
}

QColor SettingManager::textColor() {
    QVariant variant = QColor(Qt::white);
    QColor color = settings.value("textColor", variant).value<QColor>();
    return color;
}

void SettingManager::setShadowColor(QColor color) {
    settings.setValue("shadowColor", color);
}
QColor SettingManager::shadowColor() {
    QVariant variant = QColor(Qt::black);
    QColor color = settings.value("shadowColor", variant).value<QColor>();
    return color;
}

void SettingManager::setFont(QFont font) {
    settings.setValue("font", font);
}
QFont SettingManager::font() {
    QFont defaultFont = QApplication::font();
    defaultFont.setWeight(QFont::Weight::Bold);
    defaultFont.setPixelSize(40);
    return settings.value("font", defaultFont).value<QFont>();
}

void SettingManager::setDuration(int duration) {
    settings.setValue("duration", duration);
}
int SettingManager::duration() {
    return settings.value("duration", 10).toInt();
}

void SettingManager::setTextLengthLimit(int limit) {
    settings.setValue("textLengthLimit", limit);
}

int SettingManager::textLengthLimit() {
    return settings.value("textLengthLimit", 50).toInt();
}

void SettingManager::setHideUrl(bool value) {
    settings.setValue("hideUrl", value);
}
bool SettingManager::hideUrl() {
    return settings.value("hideUrl", false).toBool();
}

void SettingManager::setIgnoreContentWarning(bool value) {
    settings.setValue("ignoreContentWarning", value);
}
bool SettingManager::ignoreContentWarning() {
    return settings.value("ignoreContentWarning", false).toBool();
}

QString SettingManager::maintenanceToolPath() {
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

void SettingManager::checkForUpdate(std::function<void (bool)> callback) {
    QString maintenanceToolPath = this->maintenanceToolPath();
    QString myPath = QCoreApplication::applicationDirPath();
    QDir myDir = QDir(myPath);
    QString absPath = QDir::cleanPath(myDir.absoluteFilePath(maintenanceToolPath));
    qDebug() << "Update binary path: " << absPath;
    QProcess *checkProcess = new QProcess(this);

    connect(checkProcess, &QProcess::finished, this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "checkfinished exitCode=" << exitCode << ", exitStatus=" << exitStatus;
        QByteArray stdOut = checkProcess->readAllStandardOutput();
        qDebug() << "checkfinished>" << stdOut;
        // delete checkProcess;
        bool hasNewVersion = stdOut.contains("<updates>");
        callback(hasNewVersion);
    });

    connect(checkProcess, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        callback(false);
        // delete checkProcess;
    });

    QTimer::singleShot(0, this, [=]{
        checkProcess->start(absPath, {"check-updates"});
    });
}
