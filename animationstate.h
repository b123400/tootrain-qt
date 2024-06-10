#ifndef ANIMATIONSTATE_H
#define ANIMATIONSTATE_H

#include <QObject>
#include <QPropertyAnimation>
#include <QLabel>

class AnimationState : public QObject
{
    Q_OBJECT
public:
    explicit AnimationState(QObject *parent = nullptr);

    QPropertyAnimation *animation = nullptr;
    QLabel *target = nullptr;

signals:
};

#endif // ANIMATIONSTATE_H
