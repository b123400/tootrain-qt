#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QNetworkReply>

#include "imagemanager.h"

ImageManager::ImageManager(QObject *parent)
    : QObject{parent}
{
    networkManager = new QNetworkAccessManager(this);
}

bool ImageManager::isReady(QUrl url) {
    QString filePath = pathForUrl(url);
    return QFile::exists(filePath);
}

bool ImageManager::download(QUrl url) {
    if (isReady(url)) {
        return false;
    }
    if (inFlightUrls.contains(url)) {
        return true;
    }
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]()
            {
                // TODO: handle error
                QString err = reply->errorString();
                QByteArray buffer = reply->readAll();
                QFile file = QFile(pathForUrl(url));
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(buffer);
                }
                file.close();
                inFlightUrls.remove(url);
                emit imageFinishedDownloading(url);
            });
    inFlightUrls.insert(url);
    return true;
}

QString ImageManager::hashForUrl(QUrl url) {
    auto encoded = url.toEncoded();
    QCryptographicHash hash = QCryptographicHash(QCryptographicHash::Sha256);
    hash.addData(encoded);
    QString result = QLatin1String(hash.result().toHex());
    return result;
}

QString ImageManager::pathForUrl(QUrl url) {
    QString cacheFolder = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir = QDir(cacheFolder + QDir::separator() + "images");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    return cacheDir.absolutePath() + QDir::separator() + hashForUrl(url);
}
