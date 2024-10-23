#ifndef MISSKEYAUTHWINDOW_H
#define MISSKEYAUTHWINDOW_H

#include <QDialog>
#include <QtHttpServer>
#include <QUuid>

namespace Ui {
class MisskeyAuthWindow;
}

class MisskeyAuthWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MisskeyAuthWindow(QWidget *parent = nullptr);
    ~MisskeyAuthWindow();

public slots:
    void accept();
    void grant();
    void granted();

private:
    Ui::MisskeyAuthWindow *ui;

    QTcpServer *tcpServer = nullptr;
    QHttpServer httpServer;
    QUuid sessionId;

    void checkForSession();
};

#endif // MISSKEYAUTHWINDOW_H
