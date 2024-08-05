#include "status.h"
#include <QJsonArray>
#include <QTextDocument>

MastodonStatus::MastodonStatus( QObject *parent)
    : QObject{parent}
{
}
MastodonStatus::MastodonStatus(QJsonObject json, QObject *parent): QObject{parent}
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
    // TODO: Truncate long status
    // TODO: Make it always single line

    static QRegularExpression regex = QRegularExpression(":([a-zA-Z0-9_]+(@[a-zA-Z0-9-.]+)?):");
    QString plainTextContent = this->getPlainText();
    QList<RichTextComponent*> list;
    auto globalMatch = regex.globalMatch(plainTextContent);
    int lastStart = 0;
    while (globalMatch.hasNext()) {
        QRegularExpressionMatch match = globalMatch.next();
        QString matchedShortCode = match.captured(1);

        qsizetype start = match.capturedStart();
        qsizetype end = match.capturedEnd();
        QString before = plainTextContent.sliced(0, start);
        QString after = plainTextContent.sliced(end);

        auto textRtc = new RichTextComponent(this);
        textRtc->text = before.remove(QChar('\n'));
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
    textRtc->text = plainTextContent.sliced(lastStart).remove(QChar('\n'));
    list.append(textRtc);
    return list;
}

bool MastodonStatus::isEmojisReady() {
    for (auto emoji : emojis) {
        if (!emoji->isReady()) {
            return false;
        }
    }
    return true;
}

void MastodonStatus::downloadEmojis() {
    for (auto emoji : emojis) {
        emoji->download();
    }
}
