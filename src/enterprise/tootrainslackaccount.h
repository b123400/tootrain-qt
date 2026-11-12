#ifndef TOOTRAINSLACKACCOUNT_H
#define TOOTRAINSLACKACCOUNT_H

#include <QJsonObject>
#include <QVariantMap>
#include <optional>

#include "../account.h"

// One Slack workspace linked through the TootRain server.
//
// The app keeps one of these per workspace the user authorised, each with its
// own Slack access token, and they all share the single app session token. The
// channels the user picked are kept here as well: the server holds only what
// is being listened to right now, and a workspace switched off is listening to
// nothing, so what to put back when it is switched on again is the app's to
// remember. See `TootRainChannelSubscriptions`.
class TootRainSlackAccount : public Account
{
    Q_OBJECT
public:
    TootRainSlackAccount(const QString &teamId, const QString &teamName, const QString &teamDomain,
                         const QString &slackUserId, const QString &slackUserName, QObject *parent = nullptr);
    TootRainSlackAccount(QSettings *settings, QObject *parent = nullptr);

    QString teamId;
    QString teamName;
    QString teamDomain;
    QString slackUserId;
    QString slackUserName;

    // The server's own id for this account on the session, and the only id
    // here that is not Slack's: it is what names this one when two accounts of
    // the same workspace are on one session. 0 until the server has said.
    int linkageId = 0;

    // The channels of this workspace the user has picked: Slack's own channel
    // ids, and what each channel was called when it was picked.
    //
    // Unset for an account that has never had a set of its own, which is not
    // the same as one that has an empty set: the first takes the server's own
    // channels when the session is first asked about, the second is a
    // workspace with every channel unticked.
    std::optional<QVariantMap> channels;

    // Set when the server does not have this account on its session: an
    // administrator having taken the session away, or removed the linkage.
    // Nothing can be done with the workspace until the user authorises it
    // again.
    bool needsAuthentication = false;

    // The Slack token this workspace's API calls are made with.
    QString accessToken;

    // The accounts are made anew from the settings each time, and belong to
    // `owner`.
    static QList<TootRainSlackAccount *> allAccounts(QObject *owner);
    static TootRainSlackAccount *accountWithTeamId(const QString &teamId, QObject *owner);

    // Files this workspace in the settings, in place of whatever was saved for
    // the same workspace before.
    void save();

    // Forgets this workspace: its row in the settings with its Slack token,
    // plus the cached users and emoji that were only useful for it. The server
    // is told too, so it stops holding channels for an account nobody is
    // listening to.
    void remove();

    // Brings what is saved here into line with the server's own view of the
    // session. An account the server knows has its linkage id and its names
    // refreshed; one the server does not know has to be authorised again,
    // because that is what being logged out from the server's side looks like
    // from here.
    static void reconcile(const QJsonObject &overview);

    // Every workspace has to be authorised again: the session they were all on
    // is gone.
    static void markAllNeedingAuthentication();

    // Throws the session token away and leaves whatever is still linked here
    // waiting to be authorised on a new one.
    static void forgetSession();

    // Tells the settings window that what is saved here has changed.
    static void notifyAccountsChanged();

    QString fullUsername() override;
    void saveToSettings(QSettings *settings) override;
    QString serviceName() override;

    // The TootRain server has one connection for every workspace, so none of
    // them is a stream of its own. See `TootRainEnterpriseClient`.
    QUrl getWebSocketUrl() override { return QUrl(); }
    void connectedToWebSocket(QWebSocket *) override {}
    StreamEvent *getStreamEventFromWebSocketMessage(QString) override { return nullptr; }
};

#endif // TOOTRAINSLACKACCOUNT_H
