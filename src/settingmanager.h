#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QScreen>
#include "account.h"
#include <QColor>
#include <QFont>
#include <QProcess>

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

    void setTextColor(QColor color);
    QColor textColor();

    void setShadowColor(QColor color);
    QColor shadowColor();

    void setFont(QFont font);
    QFont font();

    void setDuration(int duration);
    int duration();

    void setTextLengthLimit(int limit);
    int textLengthLimit();

    void setHideUrl(bool value);
    bool hideUrl();

    QString maintenanceToolPath();
    void checkForUpdate(std::function<void (bool)> callback);

private:
    QSettings settings = QSettings("b123400", "TootRain-qt");

signals:
    void currentAccountChanged();
    void currentScreenChanged();
};

#endif // SETTINGMANAGER_H
