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
    void addAccountButtonClicked();
    void deleteAccountButtonClicked();
    void mastodonAccountFinished();
    void mastodonAccountAuthenticated(MastodonAccount *account);

private:
    Ui::SettingWindow *ui;

    MastodonOauthWindow *mastodonOAuthWindow;
};

#endif // SETTINGWINDOW_H
