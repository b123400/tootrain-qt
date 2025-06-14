#include "settingmanager.h"
#include <QCoreApplication>
#include <QUrl>
#include <QApplication>
#include <QDir>
#include <QTimer>
#include "mastodon/account.h"
#include "misskey/misskeyaccount.h"
#include "streammanager.h"

SettingManager::SettingManager(QObject *parent)
    : QObject{parent}
{
    QCoreApplication::setOrganizationName("b123400");
    QCoreApplication::setOrganizationDomain("b123400.net");
    QCoreApplication::setApplicationName("TootRain-qt");
}

void SettingManager::saveAccounts(QList<Account*> accounts) {
    auto currentStreamingAccounts = streamingAccounts();
    for (qsizetype i = currentStreamingAccounts.size() - 1; i >= 0; i--) {
        auto currentStreamingAccount = currentStreamingAccounts[i];
        bool found = false;
        for (auto a : accounts) {
            if (a->uuid == currentStreamingAccount->uuid) {
                found = true;
                break;
            }
        }
        if (!found) {
            // Deleted streaming account
            currentStreamingAccounts.removeAt(i);
        }
    }
    setStreamingAccounts(currentStreamingAccounts);

    settings.remove("accounts");
    settings.beginWriteArray("accounts");
    for (qsizetype i = 0; i < accounts.size(); i++) {
        settings.setArrayIndex(i);

        Account *a = accounts[i];
        a->saveToSettings(&settings);
    }
    settings.endArray();
}

void SettingManager::deleteAccountWithUuid(QString uuid) {
    auto accounts = this->getAccounts();
    int index = -1;
    for (qsizetype i = 0; i < accounts.size(); i++) {
        Account *a = accounts[i];
        if (a->uuid == uuid) {
            index = i;
            break;
        }
    }
    if (index != -1) {
        accounts.removeAt(index);
    }
    this->saveAccounts(accounts);
    qDeleteAll(accounts);
}

QList<Account*> SettingManager::getAccounts() {
    QList<Account*> accounts;
    int size = settings.beginReadArray("accounts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        Account *account = nullptr;
        QString type = settings.value("type").toString();
        if (type == "mastodon") {
            account = new MastodonAccount(&settings);
        } else if (type == "misskey") {
            account = new MisskeyAccount(&settings);
        }
        accounts.append(account);
    }
    settings.endArray();
    return accounts;
}

Account* SettingManager::accountWithUuid(QString uuid) {
    Account *result = nullptr;
    foreach (auto account, this->getAccounts()) {
        if (account->uuid == uuid) {
            result = account;
        } else {
            delete account;
        }
    }
    return result;
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

void SettingManager::setOpacity(qreal opacity) {
    settings.setValue("opacity", opacity);
    emit settingsOpacityUpdated(opacity);
}
qreal SettingManager::opacity() {
    return settings.value("opacity", 1.0).toReal();
}

QList<Account *> SettingManager::streamingAccounts() {
    // An array is saved as keys with "/" separated string with index
    // e.g. accounts/1/id, and there's a key "accounts/size" to indicate the size of the entire array
    // so to check the existence of an array, we cannot just check the raw key, but need to add the "/size" suffix
    // https://doc.qt.io/qt-6/qsettings.html#beginWriteArray
    bool hasStreamAccounts = settings.contains("streaming-accounts/size");
    if (!hasStreamAccounts) {
        // Migrate from the old single account model
        auto accounts = getAccounts();
        if (!accounts.empty()) {
            settings.beginWriteArray("streaming-accounts");
            settings.setArrayIndex(0);
            settings.setValue("uuid", accounts[0]->uuid);
            settings.endArray();
        }
        qDeleteAll(accounts);
    }
    // Migration finished

    QSet<QString> streamingUuids;
    int size = settings.beginReadArray("streaming-accounts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString uuid = settings.value("uuid").toString();
        streamingUuids.insert(uuid);
    }
    settings.endArray();

    auto accounts = getAccounts();

    QList<Account*> result;
    foreach (auto account, accounts) {
        if (streamingUuids.contains(account->uuid)) {
            result.append(account);
        } else {
            delete account;
        }
    }
    return result;
}

void SettingManager::setStreamingAccounts(QList<Account *> newAccounts) {
    // Can emit changes if needed, but we don't need it now?
    auto currentStreamingAccounts = streamingAccounts();
    QMap<QString, Account *> accountIdsToStop;
    foreach (auto currentStreamingAccount, currentStreamingAccounts) {
        accountIdsToStop.insert(currentStreamingAccount->uuid, currentStreamingAccount);
    }

    foreach (auto newAccount, newAccounts) {
        StreamManager::shared().startStreaming(newAccount); // No-op if already streaming
        accountIdsToStop.remove(newAccount->uuid);
    }
    for (auto i = accountIdsToStop.cbegin(), end = accountIdsToStop.cend(); i != end; ++i) {
        auto account = i.value();
        StreamManager::shared().stopStreaming(account);
    }
    auto newStreamingAccountUuids = StreamManager::shared().streamingAccountUuids();
    settings.remove("streaming-accounts");
    settings.beginWriteArray("streaming-accounts");
    for (qsizetype i = 0; i < newStreamingAccountUuids.size(); i++) {
        settings.setArrayIndex(i);

        QString uuid = newStreamingAccountUuids[i];
        settings.setValue("uuid", uuid);
    }
    settings.endArray();

    qDeleteAll(currentStreamingAccounts);
}

void SettingManager::startStreaming(Account *account) {
    QList<Account *> streamingAccounts = this->streamingAccounts();

    streamingAccounts.append(account);
    this->setStreamingAccounts(streamingAccounts);
    foreach (auto streamingAccount, streamingAccounts) {
        if (streamingAccount != account) {
            delete streamingAccount;
        }
    }
}

void SettingManager::stopStreaming(Account *account) {
    QList<Account *> streamingAccounts = this->streamingAccounts();
    QList<Account *> newAccounts;
    foreach (auto streamingAccount, streamingAccounts) {
        if (streamingAccount->uuid == account->uuid) {
            delete streamingAccount;
        } else {
            newAccounts.append(streamingAccount);
        }
    }
    this->setStreamingAccounts(newAccounts);
    qDeleteAll(newAccounts);
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
