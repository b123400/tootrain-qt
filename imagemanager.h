#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>

class ImageManager : public QObject
{
    Q_OBJECT
public:
    static ImageManager &shared() {static ImageManager s; return s;}
    bool isReady(QUrl url);
    bool download(QUrl url);
    QString pathForUrl(QUrl url);

private:
    explicit ImageManager(QObject *parent = nullptr);
    QNetworkAccessManager *networkManager = nullptr;
    QString hashForUrl(QUrl url);

    QSet<QUrl> inFlightUrls;

signals:
    void imageFinishedDownloading(QUrl url);

};

#endif // IMAGEMANAGER_H
