#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>
#include <QSettings>
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

private:
    QSettings settings = QSettings("TootRain-qt", "Star Runner");

signals:
    void currentAccountChanged();
};

#endif // SETTINGMANAGER_H
