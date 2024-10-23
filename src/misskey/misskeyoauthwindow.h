#ifndef MISSKEYOAUTHWINDOW_H
#define MISSKEYOAUTHWINDOW_H

#include <QDialog>
#include <QtNetworkAuth/qoauth2authorizationcodeflow.h>


namespace Ui {
class MisskeyOauthWindow;
}

class MisskeyOauthWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MisskeyOauthWindow(QWidget *parent = nullptr);
    ~MisskeyOauthWindow();

public slots:
    void accept();
    void grant();
    void granted();
    void oauthError(const QAbstractOAuth::Error err);

private:
    Ui::MisskeyOauthWindow *ui;
    QOAuth2AuthorizationCodeFlow *oauth2 = nullptr;
};

#endif // MISSKEYOAUTHWINDOW_H
