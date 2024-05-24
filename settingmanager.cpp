#include "settingmanager.h"
#include <QCoreApplication>
#include <QUrl>
#include "mastodon/account.h"

SettingManager::SettingManager(QObject *parent)
    : QObject{parent}
{
    QCoreApplication::setOrganizationName("b123400");
    QCoreApplication::setOrganizationDomain("b123400.net");
    QCoreApplication::setApplicationName("TootRain-qt");
}

void SettingManager::saveAccounts(QList<Account*> accounts) {
    settings.remove("accounts");
    settings.beginWriteArray("accounts");
    for (qsizetype i = 0; i < accounts.size(); i++) {
        settings.setArrayIndex(i);

        Account *a = accounts[i];
        settings.setValue("uuid", a->uuid);
        settings.setValue("displayName", a->displayName);
        settings.setValue("username", a->username);
        settings.setValue("id", a->id);
        settings.setValue("avatarUrl", a->avatarUrl);
        if (dynamic_cast<MastodonAccount*>(a) != nullptr) {
            MastodonAccount *ma = (MastodonAccount *)a;
            settings.setValue("type", "mastodon");
            settings.setValue("clientId", ma->app->clientId);
            settings.setValue("clientSecret", ma->app->clientSecret);
            settings.setValue("redirectUri", ma->app->redirectUri);
            settings.setValue("baseUrl", ma->app->baseUrl);
            settings.setValue("accessToken", ma->app->oauth2->token());
        }
    }
    settings.endArray();
}

QList<Account*> SettingManager::getAccounts() {
    QList<Account*> accounts;
    int size = settings.beginReadArray("accounts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        Account *account = nullptr;
        QString type = settings.value("type").toString();
        if (type == "mastodon") {
            account = new MastodonAccount(this);
            MastodonApp *app = new MastodonApp(account);
            ((MastodonAccount*)account)->app = app;
            QOAuth2AuthorizationCodeFlow *oauth2 = new QOAuth2AuthorizationCodeFlow(app);
            app->oauth2 = oauth2;
            app->clientId = settings.value("clientId").toString();
            oauth2->setClientIdentifier(app->clientId);
            oauth2->setToken(settings.value("accessToken").toString());
            app->clientSecret = settings.value("clientSecret").toString();
            app->redirectUri = settings.value("redirectUri").toString();
            app->baseUrl = settings.value("baseUrl").toUrl();
        }
        account->uuid = settings.value("uuid").toString();
        account->displayName = settings.value("displayName").toString();
        account->username = settings.value("username").toString();
        account->id = settings.value("id").toString();
        account->avatarUrl= settings.value("avatarUrl").toString();

        accounts.append(account);
    }
    settings.endArray();
    return accounts;
}

void SettingManager::clearAccounts() {
    settings.remove("accounts");
}
