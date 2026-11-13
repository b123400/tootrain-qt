#ifndef TOOTRAINSERVER_H
#define TOOTRAINSERVER_H

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>
#include <QUrlQuery>
#include <QVariantMap>
#include <functional>

// Which TootRain server this app talks to, and the one app session shared by
// every Slack workspace linked through it.
namespace TootRainServer {

// The staging server while the app is being worked on, the production one in
// what is shipped: it follows the build configuration, so a release build
// cannot be pointed at staging by accident. Either one's URLs can still be
// replaced for local development, with the `tootRainServerURL` /
// `tootRainCentrifugoURL` settings.
QUrl serverUrl();

// The bidirectional WebSocket endpoint of the Centrifugo server. Server-side
// subscriptions arrive on this connection, the connect proxy decides which.
QUrl centrifugoWebSocketUrl();

QUrl logInWithSlackUrl();

// What the app calls itself. The server records it on the session row, so it
// is sent with every request and with the Centrifugo handshake.
inline QString userAgent() { return "TootRain-qt " + QCoreApplication::applicationVersion(); }

// The app session token, empty when there is no session. The server hands it
// back from the Slack log-in redirect; everything the app asks of the server
// afterwards authenticates with it, Centrifugo's connect proxy included.
QString sessionToken();
void setSessionToken(const QString &token);

}

// The HTTP surface of the TootRain server. Everything here authenticates with
// the app session token; the session itself is created by the Slack log-in
// redirect rather than by this client.
//
// A callback gets the server's own JSON answer, or else a sentence for the
// user saying why there is none.
class TootRainServerAPI : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void (QJsonObject answer, QString error)> Callback;

    static TootRainServerAPI &shared() {static TootRainServerAPI api; return api;}

    // What this session is, as the server has it: the Slack accounts on it,
    // and what each of them is listening to. Asked for at startup, which is
    // also how the app finds out that a session an administrator took away is
    // gone: that answer is an error rather than this.
    void session(Callback callback);

    // Gives up one Slack account of this session, by the server's own linkage
    // id. `session_deleted` in the answer says the session went with it, which
    // it does when that was the last account on it.
    void deleteLinkage(int id, Callback callback);

    // What the session is listening to in one workspace: `memberships`.
    void channelMemberships(const QString &teamId, Callback callback);

    // Makes this session's channels exactly this set of
    // `{linkage_id, channel_id}` pairs, across every Slack account on it, and
    // the server works out the difference from what it holds. The whole set
    // and not a change to it, so an empty one gives everything up, which is
    // also how a workspace is switched off, its pairs simply left out.
    //
    // A pair the server will not grant does not fail this: it comes back under
    // `rejected`, and `memberships` is what the session holds afterwards.
    void replaceChannelMemberships(const QJsonArray &pairs, Callback callback);

    // The entries of `memberships` or `rejected` that belong to one Slack
    // account of the session.
    static QJsonArray ofLinkage(const QJsonArray &entries, int linkageId);

    // Slack's channel ids, and what Slack last called each of them. A channel
    // the server knows no name for is named by its id, so there is always
    // something to show.
    static QVariantMap channelNames(const QJsonArray &memberships);

    // Reads one of the server's own codes into words for the user. The same
    // codes name a refused request and a pair of a set the server would not
    // grant, so both are read here.
    static QString errorNamed(const QString &code, const QString &detail = QString());

private:
    explicit TootRainServerAPI(QObject *parent = nullptr);

    void send(const QByteArray &method, const QString &path, const QUrlQuery &query,
              const QByteArray &body, const QString &requiredArray, Callback callback);

    QNetworkAccessManager *networkManager = nullptr;
};

#endif // TOOTRAINSERVER_H
