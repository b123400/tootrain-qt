#ifndef TOOTRAINURLSCHEME_H
#define TOOTRAINURLSCHEME_H

#include <QLocalServer>
#include <QObject>
#include <QUrl>

// How the Slack log-in finds its way back into the app.
//
// The server's log-in page ends by opening `tootrain://auth?...`. The Mac app
// shows that page in a web view of its own and catches the link there; this
// app sends the user through their browser instead, so the link comes back by
// way of the system, which starts a second copy of the app with the link as
// its argument. That copy hands the link to the one already running, and
// exits.
class TootRainUrlScheme : public QObject
{
    Q_OBJECT
public:
    static TootRainUrlScheme &shared() {static TootRainUrlScheme scheme; return scheme;}

    // Whether this copy of the app was only started to deliver a link, and has
    // handed it to the copy that was already running.
    static bool handOverToRunningApp(const QStringList &arguments);

    // Tells the system that `tootrain://` links are this app's, and starts
    // taking the ones later copies of the app hand over.
    void listen();

signals:
    void urlReceived(QUrl url);

private:
    explicit TootRainUrlScheme(QObject *parent = nullptr) : QObject{parent} {}

    void registerWithSystem();

    QLocalServer server;
};

#endif // TOOTRAINURLSCHEME_H
