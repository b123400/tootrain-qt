#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QWidget>
#include <QWebSocket>

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
    void onTextMessageReceived(QString message);
    void onConnected();
    void errorOccurred(QAbstractSocket::SocketError error);

private:
    Ui::SettingWindow *ui;

    MastodonOauthWindow *mastodonOAuthWindow = nullptr;
    Account *currentAccount = nullptr;

    QWebSocket webSocket;

    void loadAccount();
};

#endif // SETTINGWINDOW_H
