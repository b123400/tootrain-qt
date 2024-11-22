#include "misskeystatus.h"
#include "misskeyemoji.h"
#include "../settingmanager.h"

MisskeyStatus::MisskeyStatus(QJsonObject json, QUrl hostUrl, QObject *parent)
    : Status{parent}
{
    id = json["id"].toString("");
    body = json["text"].toString("");
    contentWarning = json["cw"].toString();
    account = new MisskeyAccount(json["user"].toObject(), hostUrl, this);
}

QString MisskeyStatus::getText() {
    return this->body;
}

bool MisskeyStatus::isEmojisReady() {
    this->prepareEmojis();
    for (auto i = this->emojis.cbegin(), end = this->emojis.cend(); i != end; ++i) {
        auto emoji = i.value();
        if (!emoji->isReady()) {
            return false;
        }
    }
    if (this->avatarEmoji != nullptr) {
        if (!this->avatarEmoji->isReady()) {
            return false;
        }
    }
    return true;
}

void MisskeyStatus::downloadEmojis() {
    this->prepareEmojis();
    for (auto i = this->emojis.cbegin(), end = this->emojis.cend(); i != end; ++i) {
        auto emoji = i.value();
        emoji->download();
    }
    if (this->avatarEmoji != nullptr) {
        this->avatarEmoji->download();
    }
}

void MisskeyStatus::prepareEmojis() {
    if (this->preparedEmoji) return;
    this->preparedEmoji = true;
    static QRegularExpression regex = QRegularExpression(":([a-zA-Z0-9_]+(@[a-zA-Z0-9-.]+)?):");
    QString plainTextContent = this->body;

    if (SettingManager::shared().showUserAvatar()) {
        this->avatarEmoji = new UrlEmoji(this->account->avatarUrl, this);
    }

    auto globalMatch = regex.globalMatch(plainTextContent);
    int lastStart = 0;
    while (globalMatch.hasNext()) {
        QRegularExpressionMatch match = globalMatch.next();
        QString matchedShortCode = match.captured(1);

        auto emoji = new MisskeyEmoji(matchedShortCode, this->account->baseUrl, this);
        this->emojis[matchedShortCode] = emoji;
    }
}

QList<RichTextComponent*> MisskeyStatus::richTextcomponents() {
    static QRegularExpression urlRegex = QRegularExpression("https?://\\S+");
    static QRegularExpression emojiRegex = QRegularExpression(":([a-zA-Z0-9_]+(@[a-zA-Z0-9-.]+)?):");
    this->prepareEmojis();
    QString plainTextContent;
    if (!SettingManager::shared().ignoreContentWarning() && !contentWarning.isEmpty()) {
        plainTextContent = this->contentWarning;
    } else {
        plainTextContent = this->body;
    }
    if (SettingManager::shared().hideUrl()) {
        plainTextContent = plainTextContent.replace(urlRegex, "");
    }
    QList<RichTextComponent*> list;

    if (this->avatarEmoji != nullptr) {
        auto rtc = new RichTextComponent(this);
        rtc->emoji = this->avatarEmoji;
        list.append(rtc);
    }

    auto globalMatch = emojiRegex.globalMatch(plainTextContent);
    int lastStart = 0;
    while (globalMatch.hasNext()) {
        QRegularExpressionMatch match = globalMatch.next();
        QString matchedShortCode = match.captured(1);

        qsizetype start = match.capturedStart();
        qsizetype end = match.capturedEnd();
        QString before = plainTextContent.sliced(lastStart, start - lastStart);

        auto textRtc = new RichTextComponent(this);
        textRtc->text = before;
        list.append(textRtc);

        if (this->emojis.contains(matchedShortCode)) {
            auto emoji = this->emojis[matchedShortCode];
            auto emojiRtc = new RichTextComponent(this);
            emojiRtc->emoji = emoji;
            list.append(emojiRtc);
        }
        lastStart = end;
    }
    auto textRtc = new RichTextComponent(this);
    textRtc->text = plainTextContent.sliced(lastStart);
    list.append(textRtc);
    return list;
}
