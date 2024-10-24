#include "misskeystatus.h"

MisskeyStatus::MisskeyStatus(QJsonObject json, QUrl hostUrl, QObject *parent)
    : Status{parent}
{
    id = json["id"].toString("");
    body = json["text"].toString("");

    account = new MisskeyAccount(json["user"].toObject(), hostUrl, this);
}

QString MisskeyStatus::getText() {
    return this->body;
}

bool MisskeyStatus::isEmojisReady() {
    // TODO
    return true;
}

void MisskeyStatus::downloadEmojis() {
    // TODO
}

QList<RichTextComponent*> MisskeyStatus::richTextcomponents() {
    // TODO
    QList<RichTextComponent*> list;
    auto rtc = new RichTextComponent(this);
    rtc->text = this->body;
    list.append(rtc);
    return list;
}
