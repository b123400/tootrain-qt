#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QWidget>
#include <QProcess>

#include "mastodonoauthwindow.h"

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
    void mastodonAccountFinished();
    void mastodonAccountAuthenticated(MastodonAccount *account);

    void testProfile();
    void screenIndexChanged(int index);

    void checkOrUpdateClicked();
    void checkFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateCheckErrored(QProcess::ProcessError error);

private:
    Ui::SettingWindow *ui;

    MastodonOauthWindow *mastodonOAuthWindow = nullptr;
    Account *currentAccount = nullptr;
    QProcess checkProcess;
    QProcess updateProcess;

    void loadAccount();
    void loadScreens();

    QString maintenanceToolPath();
    void checkForUpdate();
    void runUpdate();
    bool hasNewVersion;
};

#endif // SETTINGWINDOW_H
