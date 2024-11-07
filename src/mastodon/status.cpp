#include "status.h"
#include <QJsonArray>
#include <QTextDocument>
#include "../settingmanager.h"

MastodonStatus::MastodonStatus( QObject *parent)
    : Status{parent}
{
}
MastodonStatus::MastodonStatus(QJsonObject json, QObject *parent): Status{parent}
{
    id = json["id"].toString();
    content = json["content"].toString();
    account = new MastodonAccount(json["account"].toObject(), this);

    if (json.contains("emojis")) {
        auto emojis = json["emojis"].toArray();
        for (auto emoji : emojis) {
            this->emojis.append(new MastodonEmoji(emoji.toObject(), this));
        }
    }
    bool with_avatar = SettingManager::shared().showUserAvatar();
    if (with_avatar) {
        auto emoji = new UrlEmoji(QUrl(this->account->avatarUrl), this);
        this->avatarEmoji = emoji;
    }
}

QString MastodonStatus::getText() {
    return this->content;
}

QString MastodonStatus::getPlainText() {
    QTextDocument document;
    document.setHtml(this->content);
    return document.toPlainText();
}

QList<RichTextComponent*> MastodonStatus::richTextcomponents() {
    static QRegularExpression regex = QRegularExpression(":([a-zA-Z0-9_]+(@[a-zA-Z0-9-.]+)?):");
    QString plainTextContent = this->getPlainText();
    QList<RichTextComponent*> list;

    if (this->avatarEmoji != nullptr) {
        auto rtc = new RichTextComponent(this);
        rtc->emoji = this->avatarEmoji;
        list.append(rtc);
    }

    auto globalMatch = regex.globalMatch(plainTextContent);
    int lastStart = 0;
    while (globalMatch.hasNext()) {
        QRegularExpressionMatch match = globalMatch.next();
        QString matchedShortCode = match.captured(1);

        qsizetype start = match.capturedStart();
        qsizetype end = match.capturedEnd();
        QString before = plainTextContent.sliced(lastStart, start - lastStart);
        // QString after = plainTextContent.sliced(end);

        auto textRtc = new RichTextComponent(this);
        textRtc->text = before;
        list.append(textRtc);

        for (auto emoji : emojis) {
            if (emoji->shortCode == matchedShortCode) {
                auto emojiRtc = new RichTextComponent(this);
                emojiRtc->emoji = emoji;
                list.append(emojiRtc);
                break;
            }
        }

        lastStart = end;
    }
    auto textRtc = new RichTextComponent(this);
    textRtc->text = plainTextContent.sliced(lastStart);
    list.append(textRtc);
    return list;
}

bool MastodonStatus::isEmojisReady() {
    for (auto emoji : emojis) {
        if (!emoji->isReady()) {
            return false;
        }
    }
    if (this->avatarEmoji != nullptr && !this->avatarEmoji->isReady()) {
        return false;
    }
    return true;
}

void MastodonStatus::downloadEmojis() {
    if (this->avatarEmoji != nullptr) {
        this->avatarEmoji->download();
    }
    for (auto emoji : emojis) {
        emoji->download();
    }
}
