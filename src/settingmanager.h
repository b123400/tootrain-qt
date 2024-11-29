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

    void setIgnoreContentWarning(bool value);
    bool ignoreContentWarning();

    void setOpacity(qreal opacity);
    qreal opacity();

    QString maintenanceToolPath();
    void checkForUpdate(std::function<void (bool)> callback);

signals:
    void settingsOpacityUpdated(qreal opacity);

private:
    // No idea why but if I use b123400 / tootrain-qt the images doesn't show
    QSettings settings = QSettings("TootRain-qt", "Star Runner");

signals:
    void currentAccountChanged();
    void currentScreenChanged();
};

#endif // SETTINGMANAGER_H
