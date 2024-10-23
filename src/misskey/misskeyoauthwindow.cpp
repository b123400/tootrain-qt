#include "misskeyoauthwindow.h"
#include "ui_misskeyoauthwindow.h"
#include <QtNetworkAuth/qoauthhttpserverreplyhandler.h>
#include <QDesktopServices>
#include "./misskeyclient.h"

MisskeyOauthWindow::MisskeyOauthWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MisskeyOauthWindow)
{
    ui->setupUi(this);
    oauth2 = new QOAuth2AuthorizationCodeFlow(this);
    setFixedSize(size());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MisskeyOauthWindow::accept);
    oauth2->setPkceMethod(QOAuth2AuthorizationCodeFlow::PkceMethod::S256);

    auto replyHandler = new QOAuthHttpServerReplyHandler(QHostAddress("localhost"), 12340, this);
    oauth2->setReplyHandler(replyHandler);
    oauth2->setScope("read:account read:notifications read:reactions read:messaging write:notes write:reactions");
    oauth2->setClientIdentifier("https://b123400.net/tootrain/misskey-landing");

    oauth2->setModifyParametersFunction([this](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
        qDebug() << "stage" << (int)stage;
        qDebug() << "params" << parameters;
        foreach (const QString &str, parameters->uniqueKeys()) {
            foreach (QVariant i, parameters->values(str))
            qDebug() << str << ':' << i;
        }
    });

    connect(oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &MisskeyOauthWindow::granted);
    connect(oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this,
            &QDesktopServices::openUrl);
    connect(oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, this, &MisskeyOauthWindow::oauthError);
}

MisskeyOauthWindow::~MisskeyOauthWindow()
{
    delete ui;
}

void MisskeyOauthWindow::accept() {
    QUrl host = QUrl(ui->hostLineEdit->text());
    qDebug() << "host: " << host;
    MisskeyClient::shared().fetchWellKnown(QUrl("https://misskey.io"), [=](MisskeyWellKnown *wk) {
        wk->setParent(this);
        oauth2->setAccessTokenUrl(wk->accessTokenUrl);
        oauth2->setAuthorizationUrl(wk->authenticateUrl);
        oauth2->grant();
    });
}
void MisskeyOauthWindow::grant() {

}
void MisskeyOauthWindow::granted() {
    auto token = oauth2->token();
    qDebug() << "got misskey token: " << token;
    // QUrl host = QUrl(ui->hostLineEdit->text());
    auto host = QUrl("https://misskey.io");
    MisskeyClient::shared().fetchAccountWithToken(host, token, [=](MisskeyAccount * account) {
        qDebug() << "got account" << account->username;
    });
}
void MisskeyOauthWindow::oauthError(const QAbstractOAuth::Error err) {
    qDebug() << "why?!";
}
