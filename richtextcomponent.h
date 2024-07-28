#ifndef RICHTEXTCOMPONENT_H
#define RICHTEXTCOMPONENT_H

#include <QObject>
#include "statusemoji.h"

class RichTextComponent : public QObject
{
    Q_OBJECT
public:
    explicit RichTextComponent(QObject *parent = nullptr);

    QString text;
    StatusEmoji *emoji = nullptr;

signals:
};

#endif // RICHTEXTCOMPONENT_H
