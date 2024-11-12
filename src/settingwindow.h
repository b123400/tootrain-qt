#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QWidget>
#include <QProcess>

#include "mastodon/mastodonoauthwindow.h"
#include "mastodon/mastodonsettingwindow.h"
#include "misskey/misskeyauthwindow.h"
#include "misskey/misskeysettingwindow.h"

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

public slots:
    void loginButtonClicked();
    void loginToMastodon(bool _checked);
    void loginToMisskey(bool _checked);

    void configureButtonClicked();
    void openMastodonSettings();
    void openMisskeySettings();

    void mastodonAccountFinished();
    void mastodonAccountAuthenticated(MastodonAccount *account);
    void mastodonSettingFinished();
    void mastodonSettingUpdated(MastodonAccount *account);

    void misskeyAccountFinished();
    void misskeyAccountAuthenticated(MisskeyAccount *account);
    void misskeySettingFinished();
    void misskeySettingUpdated(MisskeyAccount *account);

    void screenIndexChanged(int index);
    void showAvatarCheckBoxChanged(Qt::CheckState checkState);
    void textColorButtonClicked();
    void shadowColorButtonClicked();
    void fontButtonClicked();
    void textLengthLimitSpinnerChanged(int value);
    void textLengthLimitCheckBoxChanged(Qt::CheckState checked);
    void speedSliderChanged(int value);

    void checkOrUpdateClicked();
    void checkFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateCheckErrored(QProcess::ProcessError error);

private:
    Ui::SettingWindow *ui;

    MastodonOauthWindow *mastodonOAuthWindow = nullptr;
    MastodonSettingWindow *mastodonSettingWindow = nullptr;
    MisskeyAuthWindow *misskeyAuthWindow = nullptr;
    MisskeySettingWindow *misskeySettingWindow = nullptr;
    Account *currentAccount = nullptr;
    QProcess checkProcess;
    QProcess updateProcess;

    void loadAccount();
    void loadScreens();
    void reloadUIFromSettings();

    QString maintenanceToolPath();
    void checkForUpdate();
    void runUpdate();
    bool hasNewVersion;
};

#endif // SETTINGWINDOW_H
