#include "settingmanager.h"
#include <QCoreApplication>
#include <QUrl>
#include "mastodon/account.h"

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

}

bool SettingManager::showUserAvatar() {
    // TODO
    return true;
}
