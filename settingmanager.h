#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QScreen>
#include "account.h"

class SettingManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingManager(QObject *parent = nullptr);
    static SettingManager &shared() {static SettingManager man; return man;}

    void saveAccounts(QList<Account*>);
    QList<Account*> getAccounts();
    void clearAccounts();

    void setScreen(QScreen *screen);
    QScreen *getScreen();

    void setShowUserAvatar(bool showUserAvatar);
    bool showUserAvatar();

private:
    QSettings settings = QSettings("TootRain-qt", "Star Runner");

signals:
    void currentAccountChanged();
    void currentScreenChanged();
};

#endif // SETTINGMANAGER_H
