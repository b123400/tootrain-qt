#include "misskeyauthwindow.h"
#include "ui_misskeyauthwindow.h"
#include <QtNetworkAuth/qoauthhttpserverreplyhandler.h>
#include <QDesktopServices>
#include "./misskeyclient.h"

MisskeyAuthWindow::MisskeyAuthWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MisskeyAuthWindow)
{
    ui->setupUi(this);
    setFixedSize(size());

    sessionId = QUuid::createUuid();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MisskeyAuthWindow::accept);

    httpServer.route("/", [this] (const QHttpServerRequest &request) {
        auto query = request.query();
        auto code = query.queryItemValue("code");
        this->checkForSession();
        return tr("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"></head><body>Authorised, please go back to TootRain");
    });

    tcpServer = new QTcpServer();
    if (!tcpServer->listen() || !httpServer.bind(tcpServer)) {
        delete tcpServer;
    } else {
        qDebug() << "Listening on port" << tcpServer->serverPort();
    }
}

MisskeyAuthWindow::~MisskeyAuthWindow()
{
    tcpServer->close();
    delete tcpServer;
    delete ui;
}

void MisskeyAuthWindow::accept() {
    QUrlQuery params;
    params.addQueryItem("name", "TootRain");
    params.addQueryItem("icon", "https://b123400.net/tootrain/mac.png");
    params.addQueryItem("callback", "http://127.0.0.1:" + QString::number(tcpServer->serverPort()));
    params.addQueryItem("permission", "read:account,read:notifications,read:reactions,read:messaging,write:notes,write:reactions");

    auto baseUrl = QUrl(ui->hostLineEdit->text());
    baseUrl.setPath("/miauth/" + sessionId.toString(QUuid::StringFormat::WithoutBraces));
    baseUrl.setQuery(params);
    QDesktopServices::openUrl(baseUrl);
}

void MisskeyAuthWindow::checkForSession() {
    auto host = QUrl(ui->hostLineEdit->text());
    MisskeyClient::shared().fetchAuthSession(host, sessionId.toString(QUuid::StringFormat::WithoutBraces), [this](MisskeyAccount *account) {
        this->authenticated(account);
        close();
    });
}
