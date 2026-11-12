#include "tootrainurlscheme.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLocalSocket>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

// Where the running app takes links from the copies started to deliver them.
static QString serverName() {
#if defined(Q_OS_WIN)
    return "tootrain-qt-url";
#else
    // In the user's own runtime directory, so that nobody else's process can
    // stand in for the app and be handed a log-in.
    return QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + "/tootrain-qt-url";
#endif
}

bool TootRainUrlScheme::handOverToRunningApp(const QStringList &arguments) {
    if (arguments.size() < 2 || !arguments[1].startsWith("tootrain://")) return false;
    QLocalSocket socket;
    socket.connectToServer(serverName());
    if (!socket.waitForConnected(1000)) return false;
    socket.write(arguments[1].toUtf8());
    socket.waitForBytesWritten(1000);
    socket.disconnectFromServer();
    return true;
}

void TootRainUrlScheme::listen() {
    if (server.isListening()) return;
    registerWithSystem();

    // A copy of the app that is already listening keeps the links: taking the
    // name from it would bring them here, where nothing is waiting for them.
    QLocalSocket running;
    running.connectToServer(serverName());
    if (running.waitForConnected(200)) return;

    // Only this user's own processes may hand a link over.
    server.setSocketOptions(QLocalServer::UserAccessOption);
    // What a copy of the app that crashed has left behind.
    QLocalServer::removeServer(serverName());
    if (!server.listen(serverName())) {
        qDebug() << "TootRain: could not listen for tootrain:// links:" << server.errorString();
        return;
    }
    connect(&server, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket *socket = server.nextPendingConnection();
        connect(socket, &QLocalSocket::disconnected, this, [=]() {
            emit urlReceived(QUrl(QString::fromUtf8(socket->readAll())));
            socket->deleteLater();
        });
    });
}

void TootRainUrlScheme::registerWithSystem() {
    QString application = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
#if defined(Q_OS_WIN)
    QSettings classes("HKEY_CURRENT_USER\\Software\\Classes\\tootrain", QSettings::NativeFormat);
    classes.setValue("Default", "URL:TootRain");
    classes.setValue("URL Protocol", "");
    classes.setValue("shell/open/command/Default", "\"" + application + "\" \"%1\"");
#elif defined(Q_OS_LINUX)
    // A desktop entry of its own name, so that it never stands in for one the
    // app was installed with.
    QString applications = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QDir().mkpath(applications);
    QFile entry(applications + "/tootrain-qt-url-handler.desktop");
    if (!entry.open(QIODevice::WriteOnly)) return;
    entry.write("[Desktop Entry]\nType=Application\nName=TootRain\nNoDisplay=true\nExec=\""
                + application.toUtf8() + "\" %u\nMimeType=x-scheme-handler/tootrain;\n");
    entry.close();
    QProcess::startDetached("xdg-mime", {"default", "tootrain-qt-url-handler.desktop", "x-scheme-handler/tootrain"});
#else
    // shortcut: the macOS build does not register tootrain:// (that takes
    // CFBundleURLTypes in its own Info.plist), so the log-in cannot come back
    // to it; add the plist if the Qt app is ever shipped for the Mac.
    Q_UNUSED(application);
#endif
}
