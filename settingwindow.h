#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QWidget>

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

private:
    Ui::SettingWindow *ui;

    MastodonOauthWindow *mastodonOAuthWindow = nullptr;
    Account *currentAccount = nullptr;

    void loadAccount();
    void loadScreens();
};

#endif // SETTINGWINDOW_H
