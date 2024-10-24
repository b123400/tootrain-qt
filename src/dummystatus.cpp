#include "dummystatus.h"

#include "status.h"

DummyStatus::DummyStatus(QString message, QObject *parent)
    : Status{parent}
{
    this->message = message;
}

QString DummyStatus::getText() {
    return this->message;
}

bool DummyStatus::isEmojisReady() {
    return true;
}

void DummyStatus::downloadEmojis() {

}

QList<RichTextComponent*> DummyStatus::richTextcomponents() {
    RichTextComponent *c = new RichTextComponent(this);
    c->text = this->message;
    auto list = QList<RichTextComponent*>();
    list.append(c);
    return list;
}
