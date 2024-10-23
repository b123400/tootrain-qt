#ifndef MASTODONOAUTHWINDOW_H
#define MASTODONOAUTHWINDOW_H

#include <QDialog>
#include <QtNetworkAuth/qoauth2authorizationcodeflow.h>

#include "./app.h"
#include "./account.h"

namespace Ui {
class MastodonOauthWindow;
}

class MastodonOauthWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MastodonOauthWindow(QWidget *parent = nullptr);
    ~MastodonOauthWindow();

public slots:
    void myaccept();
    void grant();
    void granted();
    void oauthError(const QAbstractOAuth::Error err);
signals:
    void authenticated(MastodonAccount *account);

private:
    Ui::MastodonOauthWindow *ui;
    QOAuth2AuthorizationCodeFlow *oauth2 = nullptr;
    MastodonApp *app = nullptr;

    void updateTokenParams(QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters);
};

#endif // MASTODONOAUTHWINDOW_H
