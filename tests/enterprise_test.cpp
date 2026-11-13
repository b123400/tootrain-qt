#include <QJsonArray>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTest>
#include <QWebSocketServer>

#include "../src/enterprise/centrifugoclient.h"
#include "../src/enterprise/slackapiclient.h"

// The parts of the TootRain server integration that can be checked without a
// server: what the Centrifugo client says and does on its socket, and how
// Slack's emoji aliases are followed.
class EnterpriseTest : public QObject
{
    Q_OBJECT

private:
    static QJsonObject parse(const QString &message) {
        return QJsonDocument::fromJson(message.toUtf8()).object();
    }
    static QString json(const QJsonObject &object) {
        return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
    }

private slots:
    void emojiAliasesAreFollowed() {
        QJsonArray resolved = SlackAPIClient::resolveEmojiAliases(QJsonObject{
            {"party", "https://emoji.example/party.png"},
            {"fiesta", "alias:party"},
            {"fete", "alias:fiesta"},
            {"lost", "alias:nothing"},
            {"round", "alias:around"},
            {"around", "alias:round"},
        });
        QMap<QString, QString> urls;
        for (const QJsonValue &emoji : resolved) {
            urls[emoji["name"].toString()] = emoji["url"].toString();
        }
        QCOMPARE(urls.keys(), QStringList({"fete", "fiesta", "party"}));
        QCOMPARE(urls["fete"], QString("https://emoji.example/party.png"));
    }

    void terminalCodesAreTheOnesThatSayDoNotComeBack() {
        QVERIFY(CentrifugoClient::isTerminal(3500));
        QVERIFY(CentrifugoClient::isTerminal(4501));
        QVERIFY(!CentrifugoClient::isTerminal(3000));
        QVERIFY(!CentrifugoClient::isTerminal(4001));
        QVERIFY(!CentrifugoClient::isTerminal(1006));
    }

    // One connection from its first command to the server sending it away: the
    // headers and the connect command, a publication, a ping answered, a
    // reconnect that asks for what was missed, and a refusal that ends it.
    void centrifugoConnection() {
        QWebSocketServer server("centrifugo", QWebSocketServer::NonSecureMode);
        QVERIFY(server.listen(QHostAddress::LocalHost));
        QSignalSpy connections(&server, &QWebSocketServer::newConnection);

        CentrifugoClient client("TootRain-qt", "1.2.3");
        QSignalSpy connected(&client, &CentrifugoClient::connected);
        QSignalSpy connecting(&client, &CentrifugoClient::connecting);
        QSignalSpy disconnected(&client, &CentrifugoClient::disconnected);
        QSignalSpy subscribed(&client, &CentrifugoClient::subscribed);
        QSignalSpy unsubscribed(&client, &CentrifugoClient::unsubscribed);
        QSignalSpy publications(&client, &CentrifugoClient::publication);

        QUrl endpoint = server.serverUrl();
        endpoint.setPath("/connection/websocket");
        client.connectToServer(endpoint, {{"Authorization", "Bearer secret"}});

        // The first connection.
        QVERIFY(connections.wait());
        QWebSocket *socket = server.nextPendingConnection();
        QCOMPARE(socket->request().rawHeader("Authorization"), QByteArray("Bearer secret"));
        QSignalSpy received(socket, &QWebSocket::textMessageReceived);
        QVERIFY(received.wait());
        QJsonObject command = parse(received.takeFirst()[0].toString());
        QCOMPARE(command["id"].toInt(), 1);
        QCOMPARE(command["connect"].toObject(), (QJsonObject{{"name", "TootRain-qt"}, {"version", "1.2.3"}}));

        socket->sendTextMessage(json(QJsonObject{{"id", 1}, {"connect", QJsonObject{
            {"client", "client-1"},
            {"ping", 25},
            {"pong", true},
            {"subs", QJsonObject{
                {"sessions/1", QJsonObject{}},
                {"slack/T1/channels/C1", QJsonObject{{"recoverable", true}, {"epoch", "abcd"}, {"offset", 3}}},
            }},
        }}}));
        QVERIFY(connected.wait());
        QCOMPARE(connected.takeFirst()[0].toString(), QString("client-1"));
        QCOMPARE(subscribed.count(), 2);

        // Two replies in one frame: a publication, and a channel the server
        // has added to the live connection.
        socket->sendTextMessage(
            json(QJsonObject{{"push", QJsonObject{{"channel", "slack/T1/channels/C1"}, {"pub", QJsonObject{{"offset", 4}, {"data", QJsonObject{{"text", "hello"}}}}}}}})
            + "\n"
            + json(QJsonObject{{"push", QJsonObject{{"channel", "slack/T1/channels/C2"}, {"subscribe", QJsonObject{{"recoverable", true}, {"epoch", "efgh"}}}}}}));
        QVERIFY(publications.wait());
        QCOMPARE(publications[0][0].toString(), QString("slack/T1/channels/C1"));
        QCOMPARE(publications[0][1].toJsonObject()["text"].toString(), QString("hello"));
        QCOMPARE(subscribed.count(), 3);

        // The server's ping is answered.
        socket->sendTextMessage("{}");
        QVERIFY(received.wait());
        QCOMPARE(received.takeFirst()[0].toString(), QString("{}"));

        // A code from the half that means come back: the client reconnects,
        // and says where each channel had got to.
        socket->close(QWebSocketProtocol::CloseCode(3001), "shutdown");
        QVERIFY(connecting.wait());
        QCOMPARE(connecting[0][0].toInt(), 3001);
        QVERIFY(connections.wait());
        socket = server.nextPendingConnection();
        QSignalSpy receivedAgain(socket, &QWebSocket::textMessageReceived);
        QVERIFY(receivedAgain.wait());
        QJsonObject subs = parse(receivedAgain.takeFirst()[0].toString())["connect"].toObject()["subs"].toObject();
        QCOMPARE(subs, (QJsonObject{
            {"slack/T1/channels/C1", QJsonObject{{"recover", true}, {"epoch", "abcd"}, {"offset", 4}}},
            {"slack/T1/channels/C2", QJsonObject{{"recover", true}, {"epoch", "efgh"}}},
        }));

        // The new connection has one channel fewer, and what the other missed.
        socket->sendTextMessage(json(QJsonObject{{"id", 1}, {"connect", QJsonObject{
            {"client", "client-2"},
            {"subs", QJsonObject{
                {"slack/T1/channels/C1", QJsonObject{
                    {"recoverable", true}, {"epoch", "abcd"}, {"offset", 5},
                    {"publications", QJsonArray{QJsonObject{{"offset", 5}, {"data", QJsonObject{{"text", "missed"}}}}}},
                }},
            }},
        }}}));
        QVERIFY(connected.wait());
        QCOMPARE(unsubscribed.count(), 1);
        QCOMPARE(unsubscribed[0][0].toString(), QString("slack/T1/channels/C2"));
        QCOMPARE(publications.count(), 2);
        QCOMPARE(publications[1][1].toJsonObject()["text"].toString(), QString("missed"));

        // The server's own code for a session it does not know: the client
        // gives up, and does not come back.
        socket->close(QWebSocketProtocol::CloseCode(4501), "unauthorized");
        QVERIFY(disconnected.wait());
        QCOMPARE(disconnected[0][0].toInt(), 4501);
        QCOMPARE(disconnected[0][1].toString(), QString("unauthorized"));
        QVERIFY(!connections.wait(1500));
    }
};

QTEST_GUILESS_MAIN(EnterpriseTest)
#include "enterprise_test.moc"
