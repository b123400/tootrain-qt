#include "client.h"
#include "app.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

namespace Mastodon {
Client::Client(QObject *parent)
    : QObject{parent}
{
}

void Client::createApp(QUrl host, std::function<void (App*)> callback) {
    host.setPath("/api/v1/apps");
    auto url = host;
    auto manager = new QNetworkAccessManager();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("client_name", "Tootrain-qt");
    params.addQueryItem("redirect_uris", "http://127.0.0.1:1337/");
    params.addQueryItem("scopes", "read write");
    params.addQueryItem("website", "https://b123400.net/tootrain");

    QNetworkReply *reply = manager->post(request, params.query().toUtf8());
    connect(reply, &QNetworkReply::finished, [=]()
        {
            QString err = reply->errorString();
            QByteArray buffer = reply->readAll();
            QJsonDocument jsonDoc((QJsonDocument::fromJson(buffer)));
            QJsonObject jsonReply = jsonDoc.object();
            // qDebug() << "json:" << jsonReply;
            App *app = new App(jsonReply);
            callback(app);
        });
}

}
