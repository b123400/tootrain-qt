#include "tootrainslackaccount.h"

#include <QJsonArray>
#include <QUuid>

#include "../settingmanager.h"
#include "slackinfocache.h"
#include "tootrainenterpriseclient.h"
#include "tootrainserver.h"

TootRainSlackAccount::TootRainSlackAccount(const QString &teamId, const QString &teamName, const QString &teamDomain,
                                           const QString &slackUserId, const QString &slackUserName, QObject *parent)
    : Account{parent}
    , teamId(teamId)
    , teamName(teamName)
    , teamDomain(teamDomain)
    , slackUserId(slackUserId)
    , slackUserName(slackUserName)
{
    uuid = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
}

TootRainSlackAccount::TootRainSlackAccount(QSettings *settings, QObject *parent)
    : Account(settings, parent)
{
    teamId = settings->value("teamId").toString();
    teamName = settings->value("teamName", teamId).toString();
    teamDomain = settings->value("teamDomain").toString();
    slackUserId = settings->value("slackUserId").toString();
    slackUserName = settings->value("slackUserName").toString();
    linkageId = settings->value("linkageId").toInt();
    if (settings->contains("channels")) {
        channels = settings->value("channels").toMap();
    }
    needsAuthentication = settings->value("needsAuthentication", false).toBool();
    accessToken = settings->value("accessToken").toString();
}

void TootRainSlackAccount::saveToSettings(QSettings *settings) {
    Account::saveToSettings(settings);
    settings->setValue("type", "slack");
    settings->setValue("teamId", teamId);
    settings->setValue("teamName", teamName);
    settings->setValue("teamDomain", teamDomain);
    settings->setValue("slackUserId", slackUserId);
    settings->setValue("slackUserName", slackUserName);
    settings->setValue("linkageId", linkageId);
    if (channels) {
        settings->setValue("channels", *channels);
    }
    settings->setValue("needsAuthentication", needsAuthentication);
    settings->setValue("accessToken", accessToken);
}

QList<TootRainSlackAccount *> TootRainSlackAccount::allAccounts(QObject *owner) {
    QList<TootRainSlackAccount *> result;
    for (Account *account : SettingManager::shared().getAccounts()) {
        auto slackAccount = qobject_cast<TootRainSlackAccount *>(account);
        if (slackAccount) {
            slackAccount->setParent(owner);
            result.append(slackAccount);
        } else {
            delete account;
        }
    }
    return result;
}

TootRainSlackAccount *TootRainSlackAccount::accountWithTeamId(const QString &teamId, QObject *owner) {
    QObject others;
    for (TootRainSlackAccount *account : allAccounts(&others)) {
        if (account->teamId != teamId) continue;
        account->setParent(owner);
        return account;
    }
    return nullptr;
}

void TootRainSlackAccount::save() {
    QList<Account *> accounts = SettingManager::shared().getAccounts();
    bool isNew = true;
    for (Account *&saved : accounts) {
        auto slackAccount = qobject_cast<TootRainSlackAccount *>(saved);
        if (!slackAccount || slackAccount->teamId != teamId) continue;
        // The workspace keeps the uuid it was first saved under, which is what
        // its being switched on is filed by.
        uuid = slackAccount->uuid;
        delete saved;
        saved = this;
        isNew = false;
    }
    if (isNew) {
        accounts.append(this);
    }
    SettingManager::shared().saveAccounts(accounts);
    accounts.removeAll(this);
    qDeleteAll(accounts);
}

// The server's answer says whether the session went with the account, which it
// does when that was the last one on it, and the app follows that, since a
// session the server has let go is worth nothing here either. The call
// authenticates with the session token, so it has to go out before the token
// is thrown away.
void TootRainSlackAccount::remove() {
    int linkageId = this->linkageId;
    QString teamId = this->teamId;
    SettingManager::shared().deleteAccountWithUuid(uuid);
    SlackInfoCache::shared().removeCache(teamId);

    auto forgetSessionIfNothingIsLinked = []() {
        QObject owner;
        if (allAccounts(&owner).isEmpty()) forgetSession();
    };
    if (!linkageId) {
        // The app has never asked the server what this session is, so it has
        // no id to name the account by. All that is left is to forget it.
        forgetSessionIfNothingIsLinked();
        return;
    }
    TootRainServerAPI::shared().deleteLinkage(linkageId, [=](QJsonObject removal, QString error) {
        if (!error.isEmpty()) {
            qDebug() << "TootRain: could not tell the server to drop" << teamId << error;
            forgetSessionIfNothingIsLinked();
        } else if (removal.value("session_deleted").toBool()) {
            forgetSession();
        }
    });
}

void TootRainSlackAccount::reconcile(const QJsonObject &overview) {
    QObject owner;
    const QJsonArray linkages = overview["slack"]["linkages"].toArray();
    for (TootRainSlackAccount *account : allAccounts(&owner)) {
        // The account the app has saved for this workspace, as the server sees
        // it. Nothing is found when the server no longer has it.
        QJsonValue linkage;
        for (const QJsonValue &candidate : linkages) {
            if (candidate["workspace"]["team_id"].toString() == account->teamId
                && candidate["slack_user_id"].toString() == account->slackUserId) {
                linkage = candidate;
                break;
            }
        }
        if (linkage.isObject()) {
            account->linkageId = linkage["id"].toInt();
            account->needsAuthentication = false;
            // An account that has no set of its own yet, or one linked on
            // another computer: what the server already holds for it is what
            // the user picked, and taking it now is what keeps the next
            // statement of the whole set from giving it all up.
            if (!account->channels) {
                account->channels = TootRainServerAPI::channelNames(linkage["memberships"].toArray());
            }
            QString name = linkage["workspace"]["name"].toString();
            if (!name.isEmpty()) account->teamName = name;
            QString displayName = linkage["display_name"].toString();
            if (!displayName.isEmpty()) account->slackUserName = displayName;
        } else {
            account->needsAuthentication = true;
        }
        account->save();
    }
    notifyAccountsChanged();
}

void TootRainSlackAccount::markAllNeedingAuthentication() {
    QObject owner;
    for (TootRainSlackAccount *account : allAccounts(&owner)) {
        if (account->needsAuthentication) continue;
        account->needsAuthentication = true;
        account->save();
    }
    notifyAccountsChanged();
}

void TootRainSlackAccount::forgetSession() {
    TootRainServer::setSessionToken(QString());
    QObject owner;
    for (TootRainSlackAccount *account : allAccounts(&owner)) {
        account->linkageId = 0;
        account->save();
    }
    markAllNeedingAuthentication();
    TootRainEnterpriseClient::shared().updateConnection();
}

void TootRainSlackAccount::notifyAccountsChanged() {
    emit TootRainEnterpriseClient::shared().slackAccountsChanged();
}

QString TootRainSlackAccount::fullUsername() {
    return "(Slack) " + teamName;
}

QString TootRainSlackAccount::serviceName() {
    return "Slack";
}
